#pragma once

#include "pvm/vm/builtins/bind.hpp"
#include "pvm/std/debug.hpp"

inline void bind_everything(Func& func_chunk)
{
	bind_debug(func_chunk);
}
