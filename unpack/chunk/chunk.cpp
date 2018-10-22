#include "chunk.hpp"
#include "../except.hpp"
#include "../../config.hpp"

std::string& Chunk::read_header(Reader& reader, std::string_view expected)
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

	if (!expected.empty() && name != expected)
	{
		throw DecoderError(fmt::format("Expected chunk name '{}', got '{}'", expected, name));
	}

	return name;
}
