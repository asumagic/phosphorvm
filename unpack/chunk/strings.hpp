#ifndef STRINGS_HPP
#define STRINGS_HPP

#include "common.hpp"

struct StringDefinition
{
	std::string value;
};

struct Strg : List<StringDefinition> {};

inline void read(StringDefinition& str, Reader& reader)
{
	auto size = reader.read_pod<u32>();
	str.value = reader.read_pod_container<std::string>(size);

	fmt::print("\tString '{}'\n", str.value);
}

#endif // STRINGS_HPP
