#ifndef CODE_HPP
#define CODE_HPP

#include "common.hpp"

struct Script
{
	std::string name;
	std::vector<Block> data;
};

using Code = List<Script>;

inline void read(Script& scr, Reader& reader)
{
	scr.name = reader.read_string_reference();
	auto bytes = reader.read_pod<s32>();

	reader.read_pod<s32>(); // unknown

	auto bytecode_reader = reader;
	auto offset = reader.read_pod<s32>();

	reader.read_pod<s32>(); // unknown

	bytecode_reader.skip(offset);
	scr.data = bytecode_reader.read_pod_container<std::vector<Block>>(bytes / 4);

	fmt::print("\tCode entry for '{}'\n", scr.name);
}

#endif // CODE_HPP
