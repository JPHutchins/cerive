cmake_minimum_required(VERSION 3.25)
project(c_structs C ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ARM_CPU cortex-m3 CACHE STRING "Cortex-M core for the QEMU test build")
set(QEMU_MACHINE lm3s6965evb CACHE STRING "QEMU machine for the test run")
set(VARIANTS for hybrid handwritten CACHE STRING "Struct-API implementations to compare")

add_compile_options(-mcpu=${ARM_CPU} -mthumb
	-Wall -Wextra -Werror -Wdouble-promotion
	-ffunction-sections -fdata-sections)
add_link_options(-mcpu=${ARM_CPU} -mthumb
	-nostartfiles -Wl,--gc-sections --specs=nano.specs
	-T ${CMAKE_SOURCE_DIR}/bsp/lm3s6965.ld)

add_library(bsp OBJECT bsp/startup.c bsp/syscalls.c)

find_program(QEMU qemu-system-arm REQUIRED)

# CMake stays declarative; string transforms live in the `cstructs` python pkg.
include(cmake/cstructs.cmake)

enable_testing()

foreach(v ${VARIANTS})
	add_executable(test_${v} tests/test_shapes.c $<TARGET_OBJECTS:bsp>)
	target_include_directories(test_${v} PRIVATE derive variants/${v})
	add_test(NAME ${v}
		COMMAND ${QEMU} -M ${QEMU_MACHINE} -nographic -semihosting
			-kernel $<TARGET_FILE:test_${v}>)
	set_tests_properties(${v} PROPERTIES TIMEOUT 30)

	# Compiled only so study.c lands in compile_commands.json (clangd coverage).
	add_library(study_${v} OBJECT study/study.c)
	target_include_directories(study_${v} PRIVATE derive variants/${v})
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
