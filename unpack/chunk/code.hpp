#pragma once

#include "common.hpp"

//! Script code entry, which contains bytecode data and related metadata.
struct Script
{
	std::string name;
	std::size_t file_offset;
	std::vector<Block> data;

	std::size_t local_count = 0;

	void debug_print() const
	{
		fmt::print("\tCode entry for '{}'\n", name);
	}
};

using Code = ListChunk<Script>;

inline void user_reader(Script& script, Reader& reader)
{
	s32 bytes, offset;
	reader
		>> string_reference(script.name)
		>> bytes
		>> skip(4);

	auto bytecode_reader = reader;

	reader
		>> offset
		>> skip(4);

	script.file_offset = std::distance(bytecode_reader.begin, bytecode_reader.pos) + offset;

	bytecode_reader
		>> skip(offset)
		>> container(script.data, bytes / 4);
}
