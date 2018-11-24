#pragma once

#include "pvm/unpack/chunk/common.hpp"

struct StringDefinition
{
	std::string value;

	void debug_print() const
	{
		fmt::print("\tString '{}'\n", value);
	}
};

struct Strg : ListChunk<StringDefinition> {};

inline void user_reader(StringDefinition& def, Reader& reader)
{
	u32 size;
	reader
		>> size
		>> container(def.value, size);
}
