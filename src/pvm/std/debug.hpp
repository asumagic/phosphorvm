#pragma once

#include "pvm/interpreter/builtins/bind.hpp"

void show_message(VM& vm);

inline void bind_debug(Bindings& bindings)
{
	for (Builtin builtin : {
		Builtin{show_message, "show_message"}
	})
	{
		bindings.push(builtin);
	}
}
