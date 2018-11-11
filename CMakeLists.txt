cmake_minimum_required(VERSION 2.8)

project(phosphorvm)
add_executable(${PROJECT_NAME}
	"bytecode/disasm.cpp"
	"bytecode/names.cpp"
	"interpreter/builtins/debug.cpp"
	"interpreter/vm.cpp"
	"unpack/chunk/form.cpp"
	"unpack/mmap.cpp"

	"main.cpp"
)

set(RELEASE_OPTIONS
	"-O3"
)

target_compile_options(${PROJECT_NAME} PUBLIC
	"-std=c++1z"
	"-Wall"
	"-Wextra"
	"-Wpedantic"
	"-Wno-missing-braces"
	"-ggdb"
	$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>
)

target_link_libraries(${PROJECT_NAME}
	fmt
)
