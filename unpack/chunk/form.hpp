#pragma once

#include "common.hpp"
#include "../decode.hpp"
#include <fmt/color.h>

struct Form
{
	Gen8 gen8;
	Optn optn;
	Extn extn;
	Sond sond;
	Agrp agrp;
	Sprt sprt;
	Bgnd bgnd;
	Path path;
	Scpt scpt;
	Shdr shdr;
	Font font;
	Tmln tmln;
	Objt objt;
	Room room;
	Dafl dafl;
	Tpag tpag;
	Code code;
	Vari vari;
	Func func;
	Strg strg;
	Txtr txtr;
	Audo audo;

	void process_bytecode();
};

// TODO: perform this lazily
inline void Form::process_bytecode()
{
	auto process_references_for = [&](auto& chunk) {
		for (s32 i = 0; std::size_t(i) < chunk.definitions.size(); ++i)
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

			auto& def = chunk.definitions[i];

			auto address = def.first_address;

			if (debug_mode && def.occurrences != 0)
			{
				fmt::print(
					"Processing reference '{}' (id {}, {} occurrences)\n",
					def.name,
					i,
					def.occurrences
				);
			}

			for (unsigned j = 0; j < def.occurrences; ++j)
			{
				Script* script = find_code_from_address(address);

				if (script == nullptr)
				{
					fmt::print(
						fmt::color::yellow,
						"\tCould not find reference occurrence for '{}'\n",
						def.name
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
	};

	process_references_for(vari);
	process_references_for(func);

	for (auto& f : func.definitions)
	{
		auto pos = std::find_if(
			scpt.elements.begin(),
			scpt.elements.end(),
			[&](ScriptDefinition& def) {
				return f.name == def.name;
			}
		);

		if (pos != scpt.elements.end())
		{
			f.is_builtin = false;
			f.associated_script = &code.elements[pos->id];
		}
		else
		{
			f.is_builtin = true;
			// TODO: resolve name
		}
	}
}

inline void user_reader(Form& form, Reader& reader)
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
		case chunk_id("FUNC"): rd(form.func); break;
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
