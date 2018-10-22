cmake_minimum_required(VERSION 2.8)

project(phosphorvm)
add_executable(${PROJECT_NAME}
	"bytecode/disasm.cpp"
	"unpack/chunk/chunk.cpp"
	"unpack/decode.cpp"
	"unpack/mmap.cpp"
	"main.cpp"
)

target_compile_options(${PROJECT_NAME} PUBLIC
	"-std=c++2a"
	"-Wall"
	"-Wextra"
	"-Wpedantic"
	"-ggdb"
)

target_link_libraries(${PROJECT_NAME}
	fmt
)
