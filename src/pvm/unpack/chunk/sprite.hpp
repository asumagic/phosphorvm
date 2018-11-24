#pragma once

#include "pvm/unpack/chunk/common.hpp"

struct Sprite
{
	std::string name;
	std::int32_t width, height;
	std::int32_t texture_count;

	void debug_print() const
	{
		fmt::print("\tSprite '{}': {}x{}\n", name, width, height);
	}
};

using Sprt = ListChunk<Sprite>;

inline void user_reader(Sprite& sprite, Reader& reader)
{
	reader
		>> string_reference(sprite.name)
		>> sprite.width
		>> sprite.height;
}
