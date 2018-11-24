#pragma once

#include "pvm/interpreter/builtins/bind.hpp"
#include "pvm/std/debug.hpp"

inline void bind_everything(Bindings& bindings)
{
	bind_debug(bindings);
}
