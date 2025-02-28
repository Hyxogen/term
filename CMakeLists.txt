cmake_minimum_required(VERSION 3.5)

project(sterm LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED FALSE)
set(CMAKE_C_EXTENSIONS TRUE)
set(CMAKE_BUILD_TYPE Debug)

find_package(Python REQUIRED)

add_custom_command(
	OUTPUT	parser.c
	COMMAND	${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen.py
		${CMAKE_CURRENT_SOURCE_DIR}/state-new.txt
		${CMAKE_CURRENT_SOURCE_DIR}/include/sterm/parser.h
		${CMAKE_CURRENT_BINARY_DIR}/parser.c
	DEPENDS gen.py state-new.txt
)

add_executable(sterm
	src/main.c
	src/font.c
	src/psf.c
	src/printx.c
	src/framebuf.c
	src/vt220.c
	${CMAKE_CURRENT_BINARY_DIR}/parser.c
)

target_compile_options(sterm PRIVATE -fsanitize=address)
target_link_options(sterm PRIVATE -fsanitize=address)

target_include_directories(sterm PRIVATE include)
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL SYSTEM)
target_link_libraries(sterm PRIVATE SDL3::SDL3)
