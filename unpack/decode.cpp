#include "decode.hpp"

#include <fmt/format.h>
#include "../config.hpp"

WinFileSection& skip_chunk(WinFileSection& section, const Chunk& chunk)
{
	section.skip(chunk.length);
	return section;
}

void Gen8::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("GEN8");

	skip_chunk(file, *this);
}

void Optn::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("OPTN");

	skip_chunk(file, *this);
}

void Extn::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("EXTN");

	skip_chunk(file, *this);
}

void Sond::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("SOND");

	skip_chunk(file, *this);
}

void Agrp::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("AGRP");

	skip_chunk(file, *this);
}

void Sprite::read(WinFileSection& file)
{
	name.read(file);
	file(width)(height);
	file.skip(56);

	fmt::print("        Sprite '{}': {}x{}\n", name.str, width, height);
}

void Sprt::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("SPRT");

	auto fileback = file;

	sprites.read(file);

	file = fileback;
	skip_chunk(file, *this);
}

void Background::read(WinFileSection& file)
{
	name.read(file);
	file(unknown1)(unknown2)(unknown3)(texture_address);

	fmt::print("        Background '{}': {} {} {} {}\n", name.str, unknown1, unknown2, unknown3, texture_address);
}

void Bgnd::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("BGND");

	auto fileback = file;

	backgrounds.read(file);

	file = fileback;
	skip_chunk(file, *this);
}

void ScriptDefinition::read(WinFileSection& file)
{
	name.read(file);
	file(id);

	fmt::print("        Script #{:5<} '{}'\n", id, name.str);
}

void Path::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("PATH");

	skip_chunk(file, *this);
}

void Scpt::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("SCPT");

	auto fileback = file;

	definitions.read(file);

	file = fileback;
	skip_chunk(file, *this);
}

void Shdr::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("SHDR");

	skip_chunk(file, *this);
}

void Font::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("FONT");

	skip_chunk(file, *this);
}

void Tmln::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("TMLN");

	skip_chunk(file, *this);
}

void Objt::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("OBJT");

	skip_chunk(file, *this);
}

void Room::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("ROOM");

	skip_chunk(file, *this);
}

void Dafl::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("DAFL");

	skip_chunk(file, *this);
}

void Tpag::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("TPAG");

	skip_chunk(file, *this);
}

void Script::read(WinFileSection& file)
{
	name.read(file);

	std::int32_t bytes;
	file(bytes);

	data.resize(bytes);
	file(data.data(), bytes);

	fmt::print("        Bytecode for '{}': {} bytes\n", name.str, bytes);
}

void Code::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("CODE");

	scripts.read(file);

	skip_chunk(file, *this);
}

void Vari::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("VARI");

	skip_chunk(file, *this);
}

void Func::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("FUNC");

	skip_chunk(file, *this);
}

void Strg::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("STRG");

	skip_chunk(file, *this);
}

void Txtr::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("TXTR");

	skip_chunk(file, *this);
}

void Audo::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("AUDO");

	skip_chunk(file, *this);
}

void Form::read(WinFileSection& file)
{
	read_header(file);
	sanitize_name("FORM");

	read_all(
		file,
		gen8, optn, extn, sond, agrp, sprt, bgnd, path, scpt, shdr, font, tmln, objt, room, dafl, tpag, code, vari, func, strg,
		txtr, audo
	);
}
