#ifndef DECODE_HPP
#define DECODE_HPP

#include "chunk/background.hpp"
#include "chunk/code.hpp"
#include "chunk/metadata.hpp"
#include "chunk/sprite.hpp"
#include "chunk/strings.hpp"
#include "chunk/variable.hpp"

struct Optn {};
struct Extn {};
struct Sond {};
struct Agrp {};
struct Path {};

struct ScriptDefinition
{
	std::string name;
	std::int32_t id;
};

using Scpt = List<ScriptDefinition>;

struct Shdr {};
struct Font {};
struct Tmln {};
struct Objt {};
struct Room {};
struct Dafl {};
struct Tpag {};
struct Func {};
struct Txtr {};
struct Audo {};

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
};

void read(std::string& string, Reader& reader);

void read(Form& f, Reader& reader);

#endif // DECODE_HPP
