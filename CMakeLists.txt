cmake_minimum_required(VERSION 3.25)

if(DEFINED ZEPHYR_BASE AND COMMAND zephyr_library)
	if(CONFIG_CERIVE)
		zephyr_library()
		zephyr_library_sources(${CMAKE_CURRENT_SOURCE_DIR}/src/cerive.c)
		zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
	endif()
	return()
endif()

project(cerive C ASM)

add_library(cerive STATIC src/cerive.c)
add_library(cerive::cerive ALIAS cerive)
target_include_directories(cerive PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)
target_compile_features(cerive PUBLIC c_std_23)

if(NOT PROJECT_IS_TOP_LEVEL)
	return()
endif()

set(CERIVE_ANALYZE OFF CACHE BOOL "Enable GCC -fanalyzer static analysis")
if(CERIVE_ANALYZE)
	add_compile_options(-fanalyzer)
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VARIANTS cerive handwritten CACHE STRING "Struct-API implementations to compare")

set(WARNINGS -Wall -Wextra -Werror -Wdouble-promotion)
add_compile_options(${WARNINGS})

if(CMAKE_CROSSCOMPILING)
	set(ARM_CPU cortex-m3 CACHE STRING "Cortex-M core for the QEMU test build")
	set(QEMU_MACHINE lm3s6965evb CACHE STRING "QEMU machine for the test run")
	set(PLATFORM_OPTS -mcpu=${ARM_CPU} -mthumb -ffunction-sections -fdata-sections)
	add_compile_options(${PLATFORM_OPTS})
	add_link_options(-mcpu=${ARM_CPU} -mthumb
		-nostartfiles -Wl,--gc-sections --specs=nano.specs
		-T ${CMAKE_SOURCE_DIR}/bsp/lm3s6965.ld)
	add_library(bsp OBJECT bsp/startup.c bsp/syscalls.c)
	find_program(QEMU qemu-system-arm REQUIRED)
	set(CMAKE_CROSSCOMPILING_EMULATOR ${QEMU} -M ${QEMU_MACHINE} -nographic -semihosting -kernel)
else()
	set(PLATFORM_OPTS -fsanitize=address,undefined -fno-sanitize-recover=all)
	add_compile_options(${PLATFORM_OPTS})
	add_link_options(${PLATFORM_OPTS})
	add_library(bsp INTERFACE)
endif()
target_compile_options(cerive PRIVATE ${WARNINGS} ${PLATFORM_OPTS})

include(CheckSourceCompiles)
list(JOIN WARNINGS " " CMAKE_REQUIRED_FLAGS)
check_source_compiles(C [[
int main(void) {
	int const zero = 0;
	if (int const * const bound = &zero)
		return *bound;
	return 1;
}
]] CERIVE_COMPILER_HAS_IF_DECL)
unset(CMAKE_REQUIRED_FLAGS)
set(IF_DECL_PROBED CERIVE_COMPILER_HAS_IF_DECL=$<BOOL:${CERIVE_COMPILER_HAS_IF_DECL}>)

include(cmake/cstructs.cmake)

enable_testing()

function(add_cerive_test name source variant)
	add_executable(test_${name} ${source})
	target_link_libraries(test_${name} PRIVATE cerive bsp)
	target_include_directories(test_${name} PRIVATE variants/${variant})
	add_test(NAME ${name} COMMAND test_${name})
	set_tests_properties(${name} PROPERTIES TIMEOUT 30)
endfunction()

foreach(v ${VARIANTS})
	add_cerive_test(${v} tests/test_shapes.c ${v})

	add_library(clangd_index_study_${v} OBJECT study/study.c)
	target_link_libraries(clangd_index_study_${v} PRIVATE cerive)
	target_include_directories(clangd_index_study_${v} PRIVATE variants/${v})
endforeach()

add_cerive_test(scalars tests/test_scalars.c cerive)
add_cerive_test(debug_bounds tests/test_debug_bounds.c cerive)

add_library(test_compile_time OBJECT tests/test_compile_time.c)
target_link_libraries(test_compile_time PRIVATE cerive)
target_include_directories(test_compile_time PRIVATE variants/cerive)
target_compile_definitions(test_compile_time PRIVATE ${IF_DECL_PROBED})
add_library(test_compile_time_no_debug OBJECT tests/test_compile_time.c)
target_link_libraries(test_compile_time_no_debug PRIVATE cerive)
target_include_directories(test_compile_time_no_debug PRIVATE variants/cerive)
target_compile_definitions(test_compile_time_no_debug PRIVATE CERIVE_NO_DEBUG ${IF_DECL_PROBED})

if(CERIVE_COMPILER_HAS_IF_DECL)
	set(README_EXAMPLES ${CMAKE_BINARY_DIR}/readme_examples.c)
	add_custom_command(OUTPUT ${README_EXAMPLES}
		COMMAND ${CSTRUCTS} readme-c --readme ${CMAKE_SOURCE_DIR}/README.md --out ${README_EXAMPLES}
		DEPENDS ${CMAKE_SOURCE_DIR}/README.md ${CSTRUCTS_SRCS} ${CSTRUCTS_STAMP}
		VERBATIM)
	add_cerive_test(readme ${README_EXAMPLES} cerive)
endif()

add_library(test_rejects OBJECT tests/test_rejects.c)
target_link_libraries(test_rejects PRIVATE cerive)
set(REJECT_THIRTEEN_TRAITS_DIAGNOSTIC "macro [^ ]*CERIVE_P_over")
set(REJECT_TWO_ARITY_POINTER_DIAGNOSTIC "macro [^ ]*CERIVE_P_require_registered_type")
set(REJECT_MULTIWORD_SCALAR_DIAGNOSTIC "macro [^ ]*CERIVE_P_require_registered_type")
set(REJECT_MATCH_WITHOUT_IF_DECL_DIAGNOSTIC "require N3356 if-declarations")
set(REJECT_NULL_DEBUG_BUFFER_DIAGNOSTIC "nonnull]")
foreach(rejected
		THIRTEEN_TRAITS
		TWO_ARITY_POINTER
		MULTIWORD_SCALAR
		MATCH_WITHOUT_IF_DECL
		NULL_DEBUG_BUFFER)
	add_library(reject_${rejected} OBJECT EXCLUDE_FROM_ALL tests/test_rejects.c)
	target_link_libraries(reject_${rejected} PRIVATE cerive)
	target_compile_definitions(reject_${rejected} PRIVATE REJECT_${rejected})
	add_test(NAME rejects_${rejected}
		COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target reject_${rejected})
	set_tests_properties(rejects_${rejected} PROPERTIES
		PASS_REGULAR_EXPRESSION "${REJECT_${rejected}_DIAGNOSTIC}"
		RESOURCE_LOCK build_tree)
endforeach()

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
	add_test(NAME clang_analyze
		COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C23_STANDARD_COMPILE_OPTION} ${WARNINGS} -Qunused-arguments
			-I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR}/variants/cerive -D${IF_DECL_PROBED}
			--analyze -Xclang -analyzer-output=text -Xclang -analyzer-werror
			${CMAKE_SOURCE_DIR}/src/cerive.c
			${CMAKE_SOURCE_DIR}/tests/test_shapes.c
			${CMAKE_SOURCE_DIR}/tests/test_scalars.c
			${CMAKE_SOURCE_DIR}/tests/test_compile_time.c
			${CMAKE_SOURCE_DIR}/study/study.c)
	set_tests_properties(clang_analyze PROPERTIES TIMEOUT 120)
endif()

if(CMAKE_CROSSCOMPILING)
	include(cmake/matrix.cmake)
	foreach(v ${VARIANTS})
		add_matrix(${v} ${CMAKE_SOURCE_DIR}/study/study.c)
	endforeach()

	get_property(matrix_outputs GLOBAL PROPERTY C_STRUCTS_MATRIX_OUTPUTS)
	add_custom_target(matrix DEPENDS ${matrix_outputs})
	add_custom_target(report
		COMMAND ${CSTRUCTS} report
			--matrix-dir ${MATRIX_DIR}
			--variants ${VARIANTS}
			--cpus ${MATRIX_CPUS}
			--opts ${MATRIX_OPTS}
		DEPENDS ${matrix_outputs} ${CSTRUCTS_STAMP}
		VERBATIM USES_TERMINAL)
	add_custom_target(evidence)
	add_dependencies(evidence report)
endif()
