#ifndef CODE_HPP
#define CODE_HPP

#include "common.hpp"

struct Script
{
	std::string name;
	std::vector<char> data;
};

struct Code : List<Script> {};

inline void read(Script& scr, Reader& reader)
{
	scr.name = reader.read_string_reference();
	auto bytes = reader.read_pod<std::int32_t>();

	reader.read_pod<std::int32_t>(); // unknown

	auto bytecode_reader = reader;
	auto offset = reader.read_pod<std::int32_t>();

	reader.read_pod<std::int32_t>(); // unknown

	bytecode_reader.skip(offset);
	scr.data = bytecode_reader.read_pod_container<std::vector<char>, char>(bytes);

	fmt::print("\tCode entry for '{}'\n", scr.name);
}

#endif // CODE_HPP
