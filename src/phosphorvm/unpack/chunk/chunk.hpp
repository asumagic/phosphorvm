#pragma once

#include <fmt/core.h>
#include "phosphorvm/bytecode/types.hpp"
#include "phosphorvm/config.hpp"
#include "phosphorvm/unpack/gmreader.hpp"
#include "phosphorvm/unpack/except.hpp"

struct ChunkHeader
{
	std::string name;
	s32 length;

	void debug_print() const
	{
		fmt::print("Chunk {}: {:<10} bytes (~{:<7} KiB)\n", name, length, length / 1024);
	}
};

struct Chunk
{
	ChunkHeader header;
};

//! Returns an ID that calculated from the name, useful to handle the chunk type in a switch
//! @param v assumed to be .size() == 4
inline constexpr std::uint32_t chunk_id(std::string_view v)
{
	return v[0] | (v[1] << 8) | (v[2] << 16) | (v[3] << 24);
}

inline void user_reader(ChunkHeader& header, Reader& reader)
{
	reader
		>> container(header.name, 4)
		>> header.length;

	if (header.length < 0)
	{
		throw DecoderError{
			fmt::format(
				"Chunk has negative size '{:08x}' - corrupt file or bug",
				unsigned(header.length)
			)
		};
	}
}
