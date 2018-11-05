cmake_minimum_required(VERSION 2.8)

project(phosphorvm)
add_executable(${PROJECT_NAME}
	"bytecode/disasm.cpp"
	"interpreter/builtins/debug.cpp"
	"interpreter/vm.cpp"
	"unpack/mmap.cpp"

	"main.cpp"
)

target_compile_options(${PROJECT_NAME} PUBLIC
	"-std=c++1z"
	"-Wall"
	"-Wextra"
	"-Wpedantic"
	"-Wno-missing-braces"
	"-ggdb"
)

target_link_libraries(${PROJECT_NAME}
	fmt
)
