#ifndef LIST_HPP
#define LIST_HPP

#include "../helper.hpp"
#include <vector>

template<class T>
struct List
{
	std::vector<T> elements;
};

template<class T>
void read(List<T>& list, Reader& reader)
{
	auto address_count = reader.read_pod<std::int32_t>();

	for (std::int32_t i = 0; i < address_count; ++i)
	{
		auto element_address = reader.read_pod<std::int32_t>();
		fmt::print("    Element {:>6}/{:<6} at #{:<08x}: todo\n", i, address_count, element_address);
		list.elements.push_back(reader.read_into<T>());
	}
}

#endif // LIST_HPP
