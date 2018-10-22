cmake_minimum_required(VERSION 2.8)

project(phosphorvm)
add_executable(${PROJECT_NAME}
	"unpack/chunk/chunk.cpp"
	"unpack/decode.cpp"
	"unpack/winfile.cpp"
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
