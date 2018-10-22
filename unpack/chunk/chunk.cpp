#include "chunk.hpp"
#include "../except.hpp"
#include "../../config.hpp"

std::string& Chunk::read_header(WinFileSection& file)
{
	file(name, 4)(length);

	if (debug_mode)
	{
		fmt::print("Chunk {}: {:<10} bytes (~{:<7} KiB)\n", name, length, length / 1024);
	}

	if (length < 0)
	{
		throw DecoderError{fmt::format("Chunk has negative size '{}' - corrupt file or bug", length)};
	}

	return name;
}

void Chunk::sanitize_name(std::string_view expected)
{
	if (name != expected)
	{
		throw DecoderError(fmt::format("Expected chunk name '{}', got '{}'", expected, name));
	}
}
