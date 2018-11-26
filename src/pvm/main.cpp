#include <fmt/core.h>
#include "pvm/bc/disasm.hpp"
#include "pvm/std/everything.hpp"
#include "pvm/unpack/decode.hpp"
#include "pvm/unpack/mmap.hpp"
#include "pvm/vm/vm.hpp"

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

	bind_everything(main_form.func);

	for (auto& script : main_form.code.elements)
	{
		if constexpr (check(debug::disassemble))
		{
			Disassembler{main_form}(script);
		}

		/*if (script.name == "gml_Script_script_fibo")
		{
			VM vm{main_form};
			vm.push_stack_variable(s32(35));
			vm.run(script);

			fmt::print("FINAL STACK: \n");
			vm.print_stack_frame();
		}*/

		if (script.name == "gml_Object_object1_Create_0")
		{
			VM vm{main_form};

			//try {
				vm.run(script);
			//} catch (...) {
			//	fmt::print("oops\n");
			//}

			fmt::print("FINAL STACK: \n");
			vm.print_stack_frame();
		}
	}
}
