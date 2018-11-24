#pragma once

#include <string_view>
#include "../bytecode/types.hpp"
#include "../bytecode/enums.hpp"

class VM;

using GenericBuiltin = int(VM&);

struct Builtin
{
	GenericBuiltin* func;
	std::string_view name;
};
