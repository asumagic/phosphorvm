
#include <fmt/core.h>
#include "unpack/decode.hpp"
#include "unpack/mmap.hpp"
#include "bytecode/disasm.hpp"

int main()
{
	ReadMappedFile file{"data.win"};

	if (!file)
	{
		fmt::print("Missing 'data.win' in working directory! Failing.\n");
		return 1;
	}

	Form main_form;
	Reader{file.data()}.read_into(main_form);

	for (auto& script : main_form.code.elements)
	{
		print_disassembly(main_form, script);
	}
}
