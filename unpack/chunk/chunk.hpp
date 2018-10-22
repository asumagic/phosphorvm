#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <fmt/core.h>
#include "../helper.hpp"

struct Chunk
{
	std::string name;
	std::int32_t length;

	std::string& read_header(Reader& reader, std::string_view expected = "");
};

#endif // CHUNK_HPP
