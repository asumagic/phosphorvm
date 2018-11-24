#pragma once

#include <string_view>
#include "../bytecode/types.hpp"
#include "../bytecode/enums.hpp"

class VM;

using GenericBuiltin = void(VM&);

struct Builtin
{
	GenericBuiltin* func;
	std::string_view name;
};
