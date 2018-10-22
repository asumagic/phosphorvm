// Generic helpers for unpacking .win files

#ifndef GENERIC_HPP
#define GENERIC_HPP

#include "../reader.hpp"

inline void read(std::string& string, Reader& reader)
{
	string = reader.read_string_reference();
}

template<class T, class... Ts>
void read_into_all(Reader& reader, T& current, Ts&... to_read)
{
	reader.read_into(current);
	read_into_all(reader, to_read...);
}

inline void read_into_all(Reader&) {}

#endif // GENERIC_HPP
