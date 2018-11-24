#pragma once

#include <string_view>
#include "pvm/bc/enums.hpp"
#include "pvm/bc/types.hpp"

class VM;

using GenericBuiltin = void(VM&);

struct Builtin
{
	GenericBuiltin* func;
	std::string_view name;
};
