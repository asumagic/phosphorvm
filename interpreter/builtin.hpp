#ifndef BUILTIN_HPP
#define BUILTIN_HPP

class VM;

using Builtin = int(*)(VM&);

#endif // BUILTIN_HPP
