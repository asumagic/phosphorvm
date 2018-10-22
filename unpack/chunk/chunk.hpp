#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <fmt/core.h>
#include "../winfile.hpp"

struct Chunk
{
	std::string name;
	std::int32_t length;

	std::string& read_header(WinFileSection& file);
	void sanitize_name(std::string_view expected);
};

#endif // CHUNK_HPP
