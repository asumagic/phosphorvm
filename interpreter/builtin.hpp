#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include "../bytecode/types.hpp"
#include "../bytecode/enums.hpp"

class VM;

using Builtin = int(*)(VM&);

int show_message(VM&);

#endif // BUILTIN_HPP