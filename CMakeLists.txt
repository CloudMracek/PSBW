cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain.cmake")

project(
	psbw
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Create common library which contains things like CPP support and libc
add_library(
	common OBJECT
	lib/libc/crt0.c
	lib/libc/cxxsupport.cpp
	lib/libc/malloc.c
	lib/libc/memset.s
	lib/libc/misc.c
	lib/libc/misc.s
	lib/libc/string.c
	lib/vendor/printf.c
)

target_include_directories(
	common PUBLIC
	lib
	inc
	lib/libc
)

# Create the Game library which contains the actual game code
add_library(
	game OBJECT
	game/main.cpp
)

target_include_directories(
	game PUBLIC
	lib
	inc
	lib/libc
)

# Create the final executable of the engine and link common and game into it
add_executable(${PROJECT_NAME} src/main.cpp src/sio.cpp src/draw.cpp src/GameObject.cpp src/Sprite.cpp src/manager.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE game common)
target_include_directories(${PROJECT_NAME} PUBLIC inc)

# The final executable requires conversion to be ran on the PSX
add_custom_command(
	TARGET     ${PROJECT_NAME} POST_BUILD
	BYPRODUCTS ${PROJECT_NAME}.psexe
	COMMAND
		"${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/convertExecutable.py"
		"$<TARGET_FILE:${PROJECT_NAME}>" ${PROJECT_NAME}.psexe
	VERBATIM
)



function(addBinaryFile target name path)
	set(_file "${PROJECT_BINARY_DIR}/${target}_${name}.s")
	cmake_path(ABSOLUTE_PATH path OUTPUT_VARIABLE _path)

	file(
		CONFIGURE
		OUTPUT  "${_file}"
		CONTENT [[
.section .data.${name}, "aw"
.balign 8

.global ${name}
.type ${name}, @object
.size ${name}, (${name}_end - ${name})

${name}:
	.incbin "${_path}"
${name}_end:
		]]
		ESCAPE_QUOTES
		NEWLINE_STYLE LF
	)

	target_sources(${target} PRIVATE "${_file}")
	set_source_files_properties("${_file}" PROPERTIES OBJECT_DEPENDS "${_path}")
endfunction()

function(convertImage input bpp)
	add_custom_command(
		OUTPUT  ${ARGN}
		DEPENDS "${PROJECT_SOURCE_DIR}/${input}"
		COMMAND
			"${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/convertImage.py"
			-b ${bpp} "${PROJECT_SOURCE_DIR}/${input}" ${ARGN}
		VERBATIM
	)
endfunction()

