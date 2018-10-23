#include "decode.hpp"

#include <fmt/format.h>
#include "../config.hpp"
#include "chunk/generic.hpp"

void read(ScriptDefinition& def, Reader& reader)
{
	def.name = reader.read_string_reference();
	def.id = reader.read_pod<std::int32_t>();

	fmt::print("\tScript #{:5<} '{}'\n", def.id, def.name);
}

void read(Form& f, Reader& reader)
{
	auto orig_reader = reader;

	ChunkHeader form_header{reader};
	if (form_header.name != "FORM")
	{
		throw DecoderError{"Expected chunk FORM to begin the program"};
	}

	for(;;)
	{
		ChunkHeader header{reader};
		auto next_reader = reader;
		next_reader.skip(header.length);

		if (next_reader.distance_with(orig_reader) >= form_header.length + 4)
		{
			fmt::print("Finished reading main FORM\n");
			break;
		}

		switch (chunk_id(header.name))
		{
			case chunk_id("SCPT"): reader.read_into(f.scpt); break;
			case chunk_id("CODE"): reader.read_into(f.code); break;
			default:
				fmt::print("Unhandled chunk: {}\n", header.name);
				break;
		}
		reader = next_reader;
	}
}
