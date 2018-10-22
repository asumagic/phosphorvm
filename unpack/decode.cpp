#include "decode.hpp"

#include <fmt/format.h>
#include "../config.hpp"
#include "chunk/generic.hpp"

void read(ScriptDefinition& def, Reader& reader)
{
	def.name = reader.read_string_reference();
	def.id = reader.read_pod<std::int32_t>();

	fmt::print("\t\tScript #{:5<} '{}'\n", def.id, def.name);
}

void read(Form& f, Reader& reader)
{
	f.read_header(reader);
	if (f.name != "FORM")
	{
		throw DecoderError{"Expected chunk FORM to begin the program"};
	}

	Chunk c;

	f.gen8 = c;/*

	read_into_all(
		reader,
		f.gen8,
		f.optn,
		f.extn,
		f.optn,
		f.sond,
		f.agrp,
		f.sprt,
		f.bgnd,
		f.path,
		f.scpt,
		f.shdr,
		f.font,
		f.tmln,
		f.objt,
		f.room,
		f.dafl,
		f.tpag,
		f.code,
		f.vari,
		f.func,
		f.strg,
		f.txtr,
		f.audo
	);*/
}
