#ifndef LIST_HPP
#define LIST_HPP

#include "../reader.hpp"
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
	auto element_reader = reader;

	for (std::int32_t i = 0; i < address_count; ++i)
	{
		auto element_address = reader.read_pod<std::int32_t>();
		fmt::print("\tElement {:>6}/{:<6} at #{:<08x}: todo\n", i + 1, address_count, element_address);
		T val;
		element_reader = {reader.begin, reader.begin + element_address};
		element_reader.read_into(val);
		list.elements.push_back(val);
	}

	reader = element_reader;
}

#endif // LIST_HPP
