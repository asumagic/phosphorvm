#include "form.hpp"

#include <fmt/color.h>
#include "../../bytecode/names.hpp"

// TODO: perform this lazily
void Form::finalize_bytecode()
{
	// Caution! The process order is generally important.
	process_variables();
	process_references();
	process_functions();
}

void Form::process_variables()
{
	for (std::size_t i = 0; i < vari.definitions.size(); ++i)
	{
		auto& var = vari.definitions[i];
		auto it = special_var_names.find(var.name);
		if (it != special_var_names.end())
		{
			var.special_var = it->second;

			if (i != std::size_t(it->second))
			{
				if constexpr (check(debug::verbose_postprocess))
				{
					fmt::print(
						"Swapped VARI[{}] ({}) with VARI[{}] ({}) to map to the proper SpecialVar\n",
						i,
						var.name,
						std::size_t(it->second),
						vari.definitions[std::size_t(it->second)].name
					);
				}

				std::swap(vari.definitions[std::size_t(it->second)], var);
			}
		}
	}
}

void Form::process_references()
{
	auto process_references_for = [&](auto& chunk, auto on_reference_found) {
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

			if (check(debug::verbose_postprocess) && def.occurrences != 0)
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

				on_reference_found(def, *script);

				// Find the relevant block from the offset
				Block* block = &script->data[(address - script->file_offset) / 4];

				// Jump to the next occurrence
				address += (block[1] & 0x00FFFFFF);

				// Write the reference to the found block
				block[1] = (block[1] & 0xFF000000) | i;

				if (check(debug::verbose_postprocess))
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

	process_references_for(vari, [](VariableDefinition& def, Script& script) {
		if (def.instance_type == -1 && s32(def.unknown) >= 0)
		{
			++script.local_count;
		}
	});

	process_references_for(func, [](FunctionDefinition&, Script&) {});
}

void Form::process_functions()
{
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
		if constexpr (check(debug::verbose_unpack))
		{
			header.debug_print();
		}

		auto next_reader = reader;
		next_reader >> skip(header.length);

		auto rd = [&](auto& field) {
			field.header = header;
			reader >> field;

			if constexpr (check(debug::verbose_unpack))
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

	form.finalize_bytecode();
}
