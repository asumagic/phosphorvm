#pragma once

#include "pvm/vm/builtins/bind.hpp"

void show_message(VM& vm);

inline void bind_debug(Func& func)
{
	bind<show_message>(func, "show_message");
}
