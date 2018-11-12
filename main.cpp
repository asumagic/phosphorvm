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

	for (auto& script : main_form.code.elements)
	{
		if constexpr (check(debug::disassemble))
		{
			Disassembler{main_form}(script);
		}

		if (script.name == "gml_Script_script_fibo")
		{
			VM vm{main_form};
			vm.push_stack_variable(s32(35));
			vm.execute(script);

			fmt::print("FINAL STACK: \n");
			vm.print_stack_frame();

			return 0;
		}
	}
}
