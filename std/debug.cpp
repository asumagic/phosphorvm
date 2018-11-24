#include "debug.hpp"
#include "../interpreter/vm.hpp"

int show_message(VM& vm)
{
	// TODO. <3
	vm.print_stack_frame();

	return 0;
}
