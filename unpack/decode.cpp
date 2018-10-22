#include "decode.hpp"

#include <fmt/format.h>
#include "../config.hpp"

void read(Chunk& chunk, Reader& reader)
{
	fmt::print("Using placeholder chunk reader\n");
	chunk.read_header(reader);
	reader.skip(chunk.length);
}

void read(std::string& string, Reader& reader)
{
	string = reader.read_string();
}

void read(Sprite& spr, Reader& reader)
{
	spr.name = reader.read_string();
	spr.width = reader.read_pod<std::int32_t>();
	spr.height = reader.read_pod<std::int32_t>();

	fmt::print("\t\tSprite '{}': {}x{}\n", spr.name, spr.width, spr.height);
}

void read(Background& bg, Reader& reader)
{
	bg.name = reader.read_string();
	reader.skip(3 * 4);
	bg.texture_address = reader.read_pod<std::int32_t>();

	fmt::print("\t\tBackground '{}'", bg.name);
}

void read(ScriptDefinition& def, Reader& reader)
{
	def.name = reader.read_string();
	def.id = reader.read_pod<std::int32_t>();

	fmt::print("\t\tScript #{:5<} '{}'\n", def.id, def.name);
}

void read(Script& scr, Reader& reader)
{
	scr.name = reader.read_string();
	auto bytes = reader.read_pod<std::int32_t>();
	scr.data = reader.read_pod_container<std::vector<char>, char>(bytes);
}

void read(Form& f, Reader& reader)
{
	f.read_header(reader, "FORM");

	read_into_all(
				reader,
				f.gen8,
				f.optn,
				f.extn,
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
				);
}
