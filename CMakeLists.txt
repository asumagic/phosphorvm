cmake_minimum_required(VERSION 2.8)

project(phosphorvm)
add_executable(${PROJECT_NAME}
	"bytecode/disasm.cpp"
	"interpreter/builtins/debug.cpp"
	"interpreter/vm.cpp"
	"unpack/decode.cpp"
	"unpack/mmap.cpp"

	"main.cpp"
)

target_compile_options(${PROJECT_NAME} PUBLIC
	"-std=c++17"
	"-Wall"
	"-Wextra"
	"-Wpedantic"
	"-Wno-missing-braces"
	"-Wno-unused-but-set-parameter"
	"-ggdb"
)

target_link_libraries(${PROJECT_NAME}
	fmt
)
