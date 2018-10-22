// Generic helpers for unpacking .win files

#ifndef GENERIC_HPP
#define GENERIC_HPP

#include "../reader.hpp"

inline void read(std::string& string, Reader& reader)
{
	string = reader.read_string_reference();
}

#endif // GENERIC_HPP
