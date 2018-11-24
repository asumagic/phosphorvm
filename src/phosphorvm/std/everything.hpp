#pragma once

#include "debug.hpp"
#include "../interpreter/builtins/bind.hpp"

inline void bind_everything(Bindings& bindings)
{
	bind_debug(bindings);
}
