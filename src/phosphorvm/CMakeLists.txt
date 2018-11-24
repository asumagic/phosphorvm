add_executable(
	${PROJECT_NAME}

	"bytecode/disasm.cpp"
	"bytecode/names.cpp"
	"interpreter/builtins/bind.cpp"
	"interpreter/vm.cpp"
	"std/debug.cpp"
	"unpack/chunk/form.cpp"
	"unpack/mmap.cpp"

	"main.cpp"
)

target_include_directories(
	phosphorvm
	PRIVATE
	"${PROJECT_SRC_DIR}/src/phosphorvm"
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
	$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>
)

target_link_libraries(${PROJECT_NAME}
	fmt
)
