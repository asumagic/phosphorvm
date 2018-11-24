#pragma once

#include "phosphorvm/unpack/chunk/common.hpp"

struct Background
{
	std::string name;
	std::int32_t texture_address;

	void debug_print() const
	{
		fmt::print("\tBackground '{}'\n", name);
	}
};

using Bgnd = ListChunk<Background>;

inline void user_reader(Background& bg, Reader& reader)
{
	reader
		>> string_reference(bg.name)
		>> skip(3 * 4)
		>> bg.texture_address;
}
