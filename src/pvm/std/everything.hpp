#pragma once

#include "pvm/vm/builtins/bind.hpp"
#include "pvm/std/debug.hpp"

inline void bind_everything(Bindings& bindings)
{
	bind_debug(bindings);
}
