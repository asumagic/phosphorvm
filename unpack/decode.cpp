#include "decode.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
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

		if (next_reader.done())
		{
			fmt::print("Finished reading main FORM\n");
			break;
		}

		if (chunk_id(header.name) == chunk_id("VARI") && f.code.elements.empty())
		{
			fmt::print(fmt::color::red, "Error: CODE must appear before VARI in order to resolve variable names!\n");
		}

		switch (chunk_id(header.name))
		{
			case chunk_id("GEN8"): reader.read_into(f.gen8); break;
			case chunk_id("BGND"): reader.read_into(f.bgnd); break;
			case chunk_id("SPRT"): reader.read_into(f.sprt); break;
			case chunk_id("SCPT"): reader.read_into(f.scpt); break;
			case chunk_id("VARI"): reader.read_into(f.vari); break;
			case chunk_id("CODE"): reader.read_into(f.code); break;
			case chunk_id("STRG"): reader.read_into(f.strg); break;
			default:
				fmt::print("Unhandled chunk: {}\n", header.name);
				break;
		}

		reader = next_reader;
	}
}
