#ifndef BACKGROUND_HPP
#define BACKGROUND_HPP

#include "common.hpp"

struct Background
{
	std::string name;
	std::int32_t texture_address;
};

using Bgnd = List<Background>;

inline void read(Background& bg, Reader& reader)
{
	bg.name = reader.read_string_reference();
	reader.skip(3 * 4);
	bg.texture_address = reader.read_pod<std::int32_t>();

	fmt::print("\tBackground '{}'\n", bg.name);
}

#endif // BACKGROUND_HPP
