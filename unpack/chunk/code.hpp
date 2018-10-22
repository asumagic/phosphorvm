#ifndef CODE_HPP
#define CODE_HPP

#include "common.hpp"

struct Script
{
	std::string name;
	std::vector<char> data;
};

struct Code : Chunk
{
	List<Script> scripts;
};

inline void read(Script& scr, Reader& reader)
{
	scr.name = reader.read_string_reference();
	auto bytes = reader.read_pod<std::int32_t>();
	scr.data = reader.read_pod_container<std::vector<char>, char>(bytes);
}

inline void read(Code& code, Reader& reader)
{
	chunk_handler(code, reader, [&] {
		reader.read_into(code.scripts);
	});
}

#endif // CODE_HPP
