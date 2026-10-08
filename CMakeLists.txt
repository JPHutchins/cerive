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

set(ARM_CPU cortex-m3 CACHE STRING "Cortex-M core for the QEMU test build")
set(QEMU_MACHINE lm3s6965evb CACHE STRING "QEMU machine for the test run")
set(VARIANTS cerive handwritten CACHE STRING "Struct-API implementations to compare")

set(ARM_OPTS -mcpu=${ARM_CPU} -mthumb
	-Wall -Wextra -Werror -Wdouble-promotion
	-ffunction-sections -fdata-sections)
add_compile_options(${ARM_OPTS})
target_compile_options(cerive PRIVATE ${ARM_OPTS})
add_link_options(-mcpu=${ARM_CPU} -mthumb
	-nostartfiles -Wl,--gc-sections --specs=nano.specs
	-T ${CMAKE_SOURCE_DIR}/bsp/lm3s6965.ld)

add_library(bsp OBJECT bsp/startup.c bsp/syscalls.c)

find_program(QEMU qemu-system-arm REQUIRED)

include(cmake/cstructs.cmake)

enable_testing()

foreach(v ${VARIANTS})
	add_executable(test_${v} tests/test_shapes.c $<TARGET_OBJECTS:bsp>)
	target_link_libraries(test_${v} PRIVATE cerive)
	target_include_directories(test_${v} PRIVATE variants/${v})
	add_test(NAME ${v}
		COMMAND ${QEMU} -M ${QEMU_MACHINE} -nographic -semihosting
			-kernel $<TARGET_FILE:test_${v}>)
	set_tests_properties(${v} PROPERTIES TIMEOUT 30)

	add_library(clangd_index_study_${v} OBJECT study/study.c)
	target_link_libraries(clangd_index_study_${v} PRIVATE cerive)
	target_include_directories(clangd_index_study_${v} PRIVATE variants/${v})
endforeach()

add_executable(test_scalars tests/test_scalars.c $<TARGET_OBJECTS:bsp>)
target_link_libraries(test_scalars PRIVATE cerive)
target_include_directories(test_scalars PRIVATE variants/cerive)
add_test(NAME scalars
	COMMAND ${QEMU} -M ${QEMU_MACHINE} -nographic -semihosting
		-kernel $<TARGET_FILE:test_scalars>)
set_tests_properties(scalars PROPERTIES TIMEOUT 30)

add_library(test_compile_time OBJECT tests/test_compile_time.c)
target_link_libraries(test_compile_time PRIVATE cerive)
target_include_directories(test_compile_time PRIVATE variants/cerive)
add_library(test_compile_time_no_debug OBJECT tests/test_compile_time.c)
target_link_libraries(test_compile_time_no_debug PRIVATE cerive)
target_include_directories(test_compile_time_no_debug PRIVATE variants/cerive)
target_compile_definitions(test_compile_time_no_debug PRIVATE CERIVE_NO_DEBUG)

set(README_EXAMPLES ${CMAKE_BINARY_DIR}/readme_examples.c)
add_custom_command(OUTPUT ${README_EXAMPLES}
	COMMAND ${CSTRUCTS} readme-c --readme ${CMAKE_SOURCE_DIR}/README.md --out ${README_EXAMPLES}
	DEPENDS ${CMAKE_SOURCE_DIR}/README.md ${CSTRUCTS_SRCS} ${CSTRUCTS_STAMP}
	VERBATIM)
add_executable(test_readme ${README_EXAMPLES} $<TARGET_OBJECTS:bsp>)
target_link_libraries(test_readme PRIVATE cerive)
add_test(NAME readme
	COMMAND ${QEMU} -M ${QEMU_MACHINE} -nographic -semihosting
		-kernel $<TARGET_FILE:test_readme>)
set_tests_properties(readme PROPERTIES TIMEOUT 30)

add_library(test_rejects OBJECT tests/test_rejects.c)
target_link_libraries(test_rejects PRIVATE cerive)
set(REJECT_THIRTEEN_TRAITS_DIAGNOSTIC "in expansion of macro .CERIVE_P_over.")
set(REJECT_TWO_ARITY_POINTER_DIAGNOSTIC "in expansion of macro .CERIVE_P_require_registered_type.")
set(REJECT_MULTIWORD_SCALAR_DIAGNOSTIC "in expansion of macro .CERIVE_P_require_registered_type.")
set(REJECT_MATCH_WITHOUT_IF_DECL_DIAGNOSTIC "require N3356 if-declarations")
set(REJECT_NULL_DEBUG_BUFFER_DIAGNOSTIC "null where non-null expected")
foreach(rejected
		THIRTEEN_TRAITS
		TWO_ARITY_POINTER
		MULTIWORD_SCALAR
		MATCH_WITHOUT_IF_DECL
		NULL_DEBUG_BUFFER)
	add_test(NAME rejects_${rejected}
		COMMAND ${CMAKE_C_COMPILER} -std=c23 -Wall -Wextra -Werror -fsyntax-only
			-I${CMAKE_SOURCE_DIR}/include -DREJECT_${rejected}
			${CMAKE_SOURCE_DIR}/tests/test_rejects.c)
	set_tests_properties(rejects_${rejected} PROPERTIES
		PASS_REGULAR_EXPRESSION "${REJECT_${rejected}_DIAGNOSTIC}")
endforeach()

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

find_program(CLANG clang)
if(CLANG)
	file(GLOB debug_bounds_headers CONFIGURE_DEPENDS
		${CMAKE_SOURCE_DIR}/include/cerive/*.h ${CMAKE_SOURCE_DIR}/variants/cerive/*.h)
	set(DEBUG_BOUNDS ${CMAKE_BINARY_DIR}/test_debug_bounds.ubsan)
	add_custom_command(OUTPUT ${DEBUG_BOUNDS}
		COMMAND ${CLANG} -std=c23 -Qunused-arguments -Wall -Wextra -Werror -Wdouble-promotion
			-fsanitize=undefined -fno-sanitize-recover=all
			-I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR}/variants/cerive
			${CMAKE_SOURCE_DIR}/tests/test_debug_bounds.c ${CMAKE_SOURCE_DIR}/src/cerive.c
			-o ${DEBUG_BOUNDS}
		DEPENDS ${CMAKE_SOURCE_DIR}/tests/test_debug_bounds.c ${CMAKE_SOURCE_DIR}/src/cerive.c
			${debug_bounds_headers}
		VERBATIM)
	add_custom_target(test_debug_bounds ALL DEPENDS ${DEBUG_BOUNDS})
	add_test(NAME debug_bounds COMMAND ${DEBUG_BOUNDS})
	set(CLANG_ANALYZE_FLAGS -std=c23 -Wall -Wextra -Wdouble-promotion -Werror
		-Qunused-arguments
		-I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR}/variants/cerive
		--analyze -Xclang -analyzer-output=text)
	add_custom_target(clang-analyze
		COMMAND ${CLANG} ${CLANG_ANALYZE_FLAGS} -c
			${CMAKE_SOURCE_DIR}/src/cerive.c
			${CMAKE_SOURCE_DIR}/tests/test_shapes.c
			${CMAKE_SOURCE_DIR}/tests/test_scalars.c
			${CMAKE_SOURCE_DIR}/tests/test_compile_time.c
			${CMAKE_SOURCE_DIR}/study/study.c
		COMMENT "clang-analyze: clang --analyze over all .c files"
		VERBATIM USES_TERMINAL)
else()
	add_custom_target(clang-analyze
		COMMAND ${CMAKE_COMMAND} -E echo "clang not found (install clang)"
		VERBATIM)
endif()
