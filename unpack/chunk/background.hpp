#ifndef BACKGROUND_HPP
#define BACKGROUND_HPP

#include "common.hpp"

struct Background
{
	std::string name;
	std::int32_t texture_address;
};

struct Bgnd
{
	List<Background> backgrounds;
};

inline void read(Background& bg, Reader& reader)
{
	bg.name = reader.read_string_reference();
	reader.skip(3 * 4);
	bg.texture_address = reader.read_pod<std::int32_t>();

	fmt::print("\tBackground '{}'\n", bg.name);
}

inline void read(Bgnd& bg, Reader& reader)
{
	reader.read_into(bg.backgrounds);
}

#endif // BACKGROUND_HPP
