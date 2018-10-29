cmake_minimum_required(VERSION 2.8)

project(phosphorvm)
add_executable(${PROJECT_NAME}
	"bytecode/disasm.cpp"
	"unpack/decode.cpp"
	"unpack/mmap.cpp"
	"main.cpp"
)

target_compile_options(${PROJECT_NAME} PUBLIC
	"-std=c++2a"
	"-Wall"
	"-Wextra"
	"-Wpedantic"
	"-Wno-missing-braces"
	"-Wno-c99-extensions" # C++20 { .a = thingy } syntax
	"-ggdb"
)

target_link_libraries(${PROJECT_NAME}
	fmt
)
