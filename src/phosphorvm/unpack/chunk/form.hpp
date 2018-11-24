#pragma once

#include "phosphorvm/unpack/chunk/common.hpp"
#include "phosphorvm/unpack/decode.hpp"

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

	void finalize_bytecode();

	void process_variables();
	void process_references();
	void process_functions();
};

void user_reader(Form& form, Reader& reader);
