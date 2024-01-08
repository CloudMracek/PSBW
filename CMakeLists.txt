cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain.cmake")

project(
	psbw
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(
	PSN00BSDK_TOOLS
)

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
	lib/ps1/system.c
	lib/ps1/system.s
	lib/ps1/unhandledexc.c
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
add_executable(${PROJECT_NAME} src/main.cpp src/sio.cpp src/draw.cpp src/GameObject.cpp src/Sprite.cpp src/manager.cpp src/Controller.cpp src/Sound.cpp)
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

function(psn00bsdk_add_cd_image name image_name config_file)
	set(MKPSXISO ${PROJECT_BINARY_DIR}/tree/bin/mkpsxiso)
	cmake_path(HASH config_file _hash)

	set(_xml_file ${CMAKE_CURRENT_BINARY_DIR}/cd_image_${_hash}.xml)
	configure_file("${config_file}" ${_xml_file})

	add_custom_command(
		OUTPUT ${image_name}.bin ${image_name}.cue
		COMMAND
			${MKPSXISO} -y
			-o ${image_name}.bin -c ${image_name}.cue ${_xml_file}
		COMMENT "Building CD image ${image_name}"
		VERBATIM
		${ARGN}
	)
	add_custom_target(
		${name} ALL
		DEPENDS
			${CMAKE_CURRENT_BINARY_DIR}/${image_name}.bin
			${CMAKE_CURRENT_BINARY_DIR}/${image_name}.cue
	)
endfunction()


##SUB
include(ExternalProject)
ExternalProject_Add(
	mkpsxiso
	PREFIX subprojects
	SOURCE_DIR ${PROJECT_SOURCE_DIR}/tools/mkpsxiso
	INSTALL_DIR tree
	CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_BINARY_DIR}/tree
)

psn00bsdk_add_cd_image(
	iso      # Target name
	psbw 	 # Output file name (= template.bin + template.cue)
	iso.xml  # Path to config file
	DEPENDS psbw system.cnf mkpsxiso
)

convertImage(assets/textures/pslib.png 16 game/tex_pslib.dat)
addBinaryFile(game textureData "${PROJECT_BINARY_DIR}/game/tex_pslib.dat")
addBinaryFile(game soundData "${PROJECT_SOURCE_DIR}/assets/sounds/boing.vag")


