#include "pvm/vm/vm.hpp"
#include "pvm/std/debug.hpp"

void show_message(VM& vm)
{
	// TODO. <3
	vm.print_stack_frame();
	vm.push_stack_variable<s32>(0);
}
