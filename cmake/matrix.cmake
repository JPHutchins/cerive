set(MATRIX_OPTS "O0;O1;O2;O3;Os;Oz" CACHE STRING "Optimization levels to sweep")
set(MATRIX_CPUS "cortex-m3" CACHE STRING "Cortex-M cores to sweep (compile-only)")
set(MATRIX_DIR ${CMAKE_BINARY_DIR}/matrix CACHE INTERNAL "")

file(MAKE_DIRECTORY ${MATRIX_DIR})

find_program(ASTYLE astyle)
find_program(ARM_OBJDUMP arm-none-eabi-objdump REQUIRED)
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)
find_program(ARM_NM arm-none-eabi-nm REQUIRED)

# add_matrix(<variant> <source>): for every (cpu, opt) cell, emit the
# preprocessed expansion (.i), compiler assembly (.s), object disassembly
# (.lst) and segment sizes (.size). Compile-only, so cpu is a free axis.
function(add_matrix variant src)
	file(GLOB hdrs ${CMAKE_SOURCE_DIR}/include/cerive/*.h)
	list(APPEND hdrs ${CMAKE_SOURCE_DIR}/variants/${variant}/shapes.h)
	foreach(cpu ${MATRIX_CPUS})
		foreach(opt ${MATRIX_OPTS})
			set(stem ${MATRIX_DIR}/${variant}.${cpu}.${opt})
			set(cc ${CMAKE_C_COMPILER} -std=c23
				-Wall -Wextra -Werror -Wdouble-promotion
				-mcpu=${cpu} -mthumb -${opt}
				-ffunction-sections -fdata-sections
				-I${CMAKE_SOURCE_DIR}/include
				-I${CMAKE_SOURCE_DIR}/variants/${variant})

			if(ASTYLE)
				set(fmt COMMAND ${ASTYLE} -n -q --style=kr --pad-oper ${stem}.i)
			else()
				set(fmt)
			endif()

			add_custom_command(OUTPUT ${stem}.i
				COMMAND ${cc} -E ${src} -o ${stem}.i
				COMMAND ${CSTRUCTS} expand --file ${stem}.i
				${fmt}
				DEPENDS ${src} ${hdrs} ${CSTRUCTS_SRCS} ${CSTRUCTS_STAMP} VERBATIM)
			add_custom_command(OUTPUT ${stem}.s
				COMMAND ${cc} -S ${src} -o ${stem}.s
				DEPENDS ${src} ${hdrs} VERBATIM)
			add_custom_command(OUTPUT ${stem}.o
				COMMAND ${cc} -c ${src} -o ${stem}.o
				DEPENDS ${src} ${hdrs} VERBATIM)
			add_custom_command(OUTPUT ${stem}.lst
				COMMAND ${CSTRUCTS} capture --out ${stem}.lst -- ${ARM_OBJDUMP} -d -S ${stem}.o
				DEPENDS ${stem}.o ${CSTRUCTS_SRCS} ${CSTRUCTS_STAMP} VERBATIM)
			add_custom_command(OUTPUT ${stem}.size
				COMMAND ${CSTRUCTS} capture --out ${stem}.size -- ${ARM_SIZE} ${stem}.o
				DEPENDS ${stem}.o ${CSTRUCTS_SRCS} ${CSTRUCTS_STAMP} VERBATIM)
			add_custom_command(OUTPUT ${stem}.sym
				COMMAND ${CSTRUCTS} capture --out ${stem}.sym -- ${ARM_NM} --print-size --defined-only ${stem}.o
				DEPENDS ${stem}.o ${CSTRUCTS_SRCS} ${CSTRUCTS_STAMP} VERBATIM)

			set_property(GLOBAL APPEND PROPERTY C_STRUCTS_MATRIX_OUTPUTS
				${stem}.i ${stem}.s ${stem}.lst ${stem}.size ${stem}.sym)
		endforeach()
	endforeach()
endfunction()
