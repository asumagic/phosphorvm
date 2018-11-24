#pragma once

#include "phosphorvm/interpreter/builtins/bind.hpp"
#include "phosphorvm/std/debug.hpp"

inline void bind_everything(Bindings& bindings)
{
	bind_debug(bindings);
}
