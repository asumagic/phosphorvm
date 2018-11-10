#pragma once

#include <string>
#include <cstdlib>
#include <cxxabi.h>

template<typename T>
std::string type_name()
{
	int status;
	std::string tname = typeid(T).name();
	char* demangled_name = abi::__cxa_demangle(tname.c_str(), nullptr, nullptr, &status);
	if(status == 0)
	{
		tname = demangled_name;
		std::free(demangled_name);
	}
	return tname;
}
