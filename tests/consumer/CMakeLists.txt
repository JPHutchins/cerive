cmake_minimum_required(VERSION 3.25)
project(cerive_consumer C)

set(CERIVE_DIR "" CACHE PATH "The cerive tree to consume")
if(NOT IS_DIRECTORY "${CERIVE_DIR}/include/cerive")
	message(FATAL_ERROR "CERIVE_DIR must point at a cerive tree, not '${CERIVE_DIR}'")
endif()

enable_testing()
add_subdirectory(${CERIVE_DIR} cerive)

add_executable(consumer ../test_shapes.c)
target_link_libraries(consumer PRIVATE cerive::cerive)
target_include_directories(consumer PRIVATE ../../variants/cerive)
target_compile_options(consumer PRIVATE -Wall -Wextra -Werror -Wdouble-promotion)
install(TARGETS consumer)
add_test(NAME consumer COMMAND consumer)
