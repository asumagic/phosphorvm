#pragma once

#include <string_view>
#include "phosphorvm/bytecode/enums.hpp"
#include "phosphorvm/bytecode/types.hpp"

class VM;

using GenericBuiltin = void(VM&);

struct Builtin
{
	GenericBuiltin* func;
	std::string_view name;
};
