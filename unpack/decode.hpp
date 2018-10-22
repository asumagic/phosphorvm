#ifndef DECODE_HPP
#define DECODE_HPP

#include "chunk/background.hpp"
#include "chunk/code.hpp"
#include "chunk/sprite.hpp"

struct Gen8 : Chunk {};
struct Optn : Chunk {};
struct Extn : Chunk {};
struct Sond : Chunk {};
struct Agrp : Chunk {};
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

void read(Chunk& chunk, Reader& reader);

void read(std::string& string, Reader& reader);


void read(Form& f, Reader& reader);

#endif // DECODE_HPP
