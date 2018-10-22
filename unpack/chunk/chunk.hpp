#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <fmt/core.h>
#include "../reader.hpp"
#include "../except.hpp"
#include "../../config.hpp"

struct Chunk
{
	std::string name;
	std::int32_t length;

	void read_header(Reader& reader, std::string_view expected = "");
};

inline void Chunk::read_header(Reader& reader, std::string_view expected)
{
	name = reader.read_string(4);
	length = reader.read_pod<std::int32_t>();

	if (debug_mode)
	{
		fmt::print("Chunk {}: {:<10} bytes (~{:<7} KiB)\n", name, length, length / 1024);
	}

	if (length < 0)
	{
		for(;;);
		throw DecoderError{fmt::format("Chunk has negative size '{}' - corrupt file or bug", length)};
	}

	if (!expected.empty() && name != expected)
	{
		throw DecoderError(fmt::format("Expected chunk name '{}', got '{}'", expected, name));
	}
}

//! This helper function has two purposes:
//! - It reads the header for a chunk.
//! - It simplifies writing chunk readers when you cannot deduce the size of a chunk without its header.
//!   For example, when reading a list the elements may have a variable size. You'd need to always entirely
//!   read elements, which is inconvenient during development.
template<class Func>
void chunk_handler(Chunk& chunk, Reader& reader, const Func& func)
{
	chunk.read_header(reader);
	auto old_reader = reader;
	func();
	reader = old_reader;
	reader.skip(chunk.length);
}

inline void read(Chunk& chunk, Reader& reader)
{
	fmt::print("Using placeholder chunk reader\n");
	chunk.read_header(reader);
	reader.skip(chunk.length);
}

#endif // CHUNK_HPP
