#ifndef DECODE_HPP
#define DECODE_HPP

#include "chunk/chunk.hpp"

#include <fmt/core.h>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "winfile.hpp"

template<class T>
struct List
{
	std::vector<T> elements;

	void read(WinFileSection& file)
	{
		std::int32_t address_count;
		file(address_count);

		auto reader = file;

		for (std::int32_t i = 0; i < address_count; ++i)
		{
			std::int32_t element_address;
			file(element_address);

			fmt::print("    Element {:>6}/{:<6} at #{:<08x}: todo\n", i, address_count, element_address);

			T value;
			reader = file.read_at(element_address);
			value.read(reader);
			elements.push_back(value);
		}

		file = reader;
	}
};

struct String
{
	std::string str;

	void read(WinFileSection& file)
	{
		std::int32_t address;
		file(address);

		// TODO: this is very bad
		auto str_section = file.read_at(address);
		char c;
		std::vector<char> v;
		do
		{
			str_section(c);
			v.push_back(c);
		} while (c != '\0');

		str = {v.begin(), v.end() - 1};
	}
};

WinFileSection& skip_chunk(WinFileSection& section, const Chunk& chunk);

template<class T, class... Ts>
void read_all(WinFileSection& file, T& current, Ts&... to_read)
{
	current.read(file);
	read_all(file, to_read...);
}

inline void read_all(WinFileSection&) {}

struct Gen8 : Chunk
{
	void read(WinFileSection& file);
};

struct Optn : Chunk
{
	void read(WinFileSection& file);
};

struct Extn : Chunk
{
	void read(WinFileSection& file);
};

struct Sond : Chunk
{
	void read(WinFileSection& file);
};

struct Agrp : Chunk
{
	void read(WinFileSection& file);
};

struct Sprite
{
	String name;
	std::int32_t width, height;
	std::int32_t texture_count;

	void read(WinFileSection& file);
};

struct Sprt : Chunk
{
	List<Sprite> sprites;

	void read(WinFileSection& file);
};

struct Background
{
	String name;
	std::int32_t unknown1, unknown2, unknown3, texture_address;

	void read(WinFileSection& file);
};

struct Bgnd : Chunk
{
	List<Background> backgrounds;

	void read(WinFileSection& file);
};

struct Path : Chunk
{
	void read(WinFileSection& file);
};

struct ScriptDefinition
{
	String name;
	std::int32_t id;

	void read(WinFileSection& file);
};

struct Scpt : Chunk
{
	List<ScriptDefinition> definitions;

	void read(WinFileSection& file);
};

struct Shdr : Chunk
{
	void read(WinFileSection& file);
};

struct Font : Chunk
{
	void read(WinFileSection& file);
};

struct Tmln : Chunk
{
	void read(WinFileSection& file);
};

struct Objt : Chunk
{
	void read(WinFileSection& file);
};

struct Room : Chunk
{
	void read(WinFileSection& file);
};

struct Dafl : Chunk
{
	void read(WinFileSection& file);
};

struct Tpag : Chunk
{
	void read(WinFileSection& file);
};

struct Script
{
	String name;
	std::vector<char> data;

	void read(WinFileSection& file);
};

struct Code : Chunk
{
	List<Script> scripts;

	void read(WinFileSection& file);
};

struct Vari : Chunk
{
	void read(WinFileSection& file);
};

struct Func : Chunk
{
	void read(WinFileSection& file);
};

struct Strg : Chunk
{
	void read(WinFileSection& file);
};

struct Txtr : Chunk
{
	void read(WinFileSection& file);
};

struct Audo : Chunk
{
	void read(WinFileSection& file);
};

struct Form : Chunk
{
	void read(WinFileSection& file);

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
};

#endif // DECODE_HPP
