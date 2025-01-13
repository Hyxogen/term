cmake_minimum_required(VERSION 3.5)

project(sterm LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS FALSE)
set(CMAKE_BUILD_TYPE Debug)

add_executable(sterm
	src/main.c
	src/sterm.c
	src/font.c
	src/psf.c
	src/printx.c
	src/framebuf.c
)

target_compile_options(sterm PRIVATE -fsanitize=address)
target_link_options(sterm PRIVATE -fsanitize=address)

target_include_directories(sterm PRIVATE include)
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL SYSTEM)
target_link_libraries(sterm PRIVATE SDL3::SDL3)
