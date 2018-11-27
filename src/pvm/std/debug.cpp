#include "pvm/std/debug.hpp"
#include "pvm/vm/typecheck.hpp"
#include "pvm/vm/vm.hpp"

void show_message(VM& vm)
{
	vm.pop_dispatch([&](auto v) {
		if constexpr (type_check<decltype(vm.value(v))>(TypeCheck::Printable))
		{
			fmt::print("APP: {}\n", vm.value(v));
		}
		else
		{
			vm.type_error();
		}
	});
	vm.push_stack_variable<s32>(0);
}
