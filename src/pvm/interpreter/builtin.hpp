#pragma once

#include <string_view>
#include "pvm/bytecode/enums.hpp"
#include "pvm/bytecode/types.hpp"

class VM;

using GenericBuiltin = void(VM&);

struct Builtin
{
	GenericBuiltin* func;
	std::string_view name;
};
