#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <fmt/core.h>
#include "../reader.hpp"
#include "../except.hpp"
#include "../../config.hpp"

struct ChunkHeader
{
	std::string name;
	std::int32_t length;

	ChunkHeader(Reader& reader);
};

//! Returns an ID that calculated from the name, useful to handle the chunk type in a switch
//! @param v assumed to be .size() == 4
inline constexpr std::uint32_t chunk_id(std::string_view v)
{
	return v[0] | (v[1] << 8) | (v[2] << 16) | (v[3] << 24);
}

inline ChunkHeader::ChunkHeader(Reader& reader)
{
	name = reader.read_string(4);
	length = reader.read_pod<std::int32_t>();

	if (debug_mode)
	{
		fmt::print("Chunk {}: {:<10} bytes (~{:<7} KiB)\n", name, length, length / 1024);
	}

	if (length < 0)
	{
		throw DecoderError{fmt::format("Chunk has negative size '{}' - corrupt file or bug", length)};
	}
}

#endif // CHUNK_HPP
