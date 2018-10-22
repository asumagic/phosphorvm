#ifndef DECODE_HPP
#define DECODE_HPP

#include "chunk/chunk.hpp"
#include "chunk/list.hpp"
#include "helper.hpp"

#include <fmt/core.h>
#include <string_view>
#include <unordered_map>
#include <vector>

struct Gen8 : Chunk {};
struct Optn : Chunk {};
struct Extn : Chunk {};
struct Sond : Chunk {};
struct Agrp : Chunk {};

struct Sprite
{
	std::string name;
	std::int32_t width, height;
	std::int32_t texture_count;
};

struct Sprt : Chunk
{
	List<Sprite> sprites;
};

struct Background
{
	std::string name;
	std::int32_t texture_address;
};

struct Bgnd : Chunk
{
	List<Background> backgrounds;
};

struct Path : Chunk {};

struct ScriptDefinition
{
	std::string name;
	std::int32_t id;
};

struct Scpt : Chunk
{
	List<ScriptDefinition> definitions;
};

struct Shdr : Chunk {};
struct Font : Chunk {};
struct Tmln : Chunk {};
struct Objt : Chunk {};
struct Room : Chunk {};
struct Dafl : Chunk {};
struct Tpag : Chunk {};

struct Script
{
	std::string name;
	std::vector<char> data;
};

struct Code : Chunk
{
	List<Script> scripts;
};

struct Vari : Chunk {};
struct Func : Chunk {};
struct Strg : Chunk {};
struct Txtr : Chunk {};
struct Audo : Chunk {};

struct Form : Chunk
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
};

template<class T, class... Ts>
void read_into_all(Reader& reader, T& current, Ts&... to_read)
{
	reader.read_into(current);
	read_into_all(reader, to_read...);
}

inline void read_into_all(Reader&) {}

void read(Chunk& chunk, Reader& reader);

void read(std::string& string, Reader& reader);

void read(Sprite& spr, Reader& reader);

void read(Background& bg, Reader& reader);

void read(ScriptDefinition& def, Reader& reader);

void read(Script& scr, Reader& reader);

void read(Form& f, Reader& reader);

#endif // DECODE_HPP
