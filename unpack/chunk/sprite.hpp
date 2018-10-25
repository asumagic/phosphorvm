#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "common.hpp"

struct Sprite
{
	std::string name;
	std::int32_t width, height;
	std::int32_t texture_count;
};

struct Sprt : List<Sprite> {};

inline void read(Sprite& spr, Reader& reader)
{
	spr.name = reader.read_string_reference();
	spr.width = reader.read_pod<std::int32_t>();
	spr.height = reader.read_pod<std::int32_t>();

	fmt::print("\tSprite '{}': {}x{}\n", spr.name, spr.width, spr.height);
}

#endif // SPRITE_HPP
