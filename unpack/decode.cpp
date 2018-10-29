#include "decode.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include "../config.hpp"

void Form::process_bytecode()
{
	for (s32 i = 0; std::size_t(i) < vari.definitions.size(); ++i)
	{
		auto find_code_from_address = [this](auto address) -> Script* {
			for (auto& script : code.elements)
			{
				if (address >= script.file_offset
				 && address < script.file_offset + script.data.size() * 4)
				{
					return &script;
				}
			}

			return nullptr;
		};

		auto& var = vari.definitions[i];

		auto address = var.first_address;

		if (debug_mode && var.occurrences != 0)
		{
			fmt::print(
				"Processing variable '{}' (id {}, {} occurrences)\n",
				var.name,
				i,
				var.occurrences
			);
		}

		for (unsigned j = 0; j < var.occurrences; ++j)
		{
			Script* script = find_code_from_address(address);

			if (script == nullptr)
			{
				fmt::print(
					fmt::color::yellow,
					"\tCould not find variable occurrence for '{}'\n",
					var.name
				);
				break;
			}

			Block* block = &script->data[(address - script->file_offset) / 4];

			address += (block[1] & 0x00FFFFFF);
			block[1] = (block[1] & 0xFF000000) | i;

			if (debug_mode)
			{
				fmt::print(
					"\tOverriden in '{}'\n",
					script->name,
					i
				);
			}
		}
	}
}

void user_reader(Form& form, Reader& reader)
{
	const ChunkHeader form_header = reader();

	if (form_header.name != "FORM")
	{
		throw DecoderError{"Bad file: Missing FORM main chunk"};
	}

	while (reader.pos != reader.end)
	{
		const ChunkHeader header = reader();
		if (debug_mode)
		{
			header.debug_print();
		}

		auto next_reader = reader;
		next_reader >> skip(header.length);

		auto rd = [&](auto& field) {
			field.header = header;
			reader >> field;

			if (debug_mode)
			{
				field.debug_print();
			}
		};

		switch (chunk_id(header.name))
		{
		case chunk_id("GEN8"): rd(form.gen8); break;
		case chunk_id("BGND"): rd(form.bgnd); break;
		case chunk_id("SPRT"): rd(form.sprt); break;
		case chunk_id("SCPT"): rd(form.scpt); break;
		case chunk_id("VARI"): rd(form.vari); break;
		case chunk_id("CODE"): rd(form.code); break;
		case chunk_id("STRG"): rd(form.strg); break;
		default:
			fmt::print("Unhandled chunk: {}\n", header.name);
			break;
		}

		reader = next_reader;
	}

	form.process_bytecode();
}
