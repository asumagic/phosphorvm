#pragma once

#include "../bytecode/types.hpp"
#include "../bytecode/enums.hpp"

struct VM;

using Builtin = int(*)(VM&);

int show_message(VM&);
