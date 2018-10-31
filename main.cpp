#include <fmt/core.h>
#include "unpack/decode.hpp"
#include "unpack/mmap.hpp"
#include "bytecode/disasm.hpp"
#include "interpreter/vm.hpp"

int main()
{
	ReadMappedFile file{"data.win"};

	if (!file)
	{
		fmt::print("Missing 'data.win' in working directory! Failing.\n");
		return 1;
	}

	Form main_form;
	Reader reader{file.data(), file.data() + file.size()};
	reader >> main_form;

	if (debug_mode)
	{
		Disassembler disasm{main_form};

		for (auto& script : main_form.code.elements)
		{
			if (script.name == "gml_Script_script_fibo")
			{
				VM vm{main_form};
				vm.execute(script);

				return 0;
			}

			disasm(script);
		}
	}
}
