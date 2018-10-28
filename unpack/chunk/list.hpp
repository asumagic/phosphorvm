#ifndef LIST_HPP
#define LIST_HPP

#include "../reader.hpp"
#include <vector>

#include <chrono>
#include <thread>

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

		T val;
		element_reader = {reader, element_address};
		element_reader.read_into(val);
		list.elements.push_back(val);
	}

	reader = element_reader;
}

#endif // LIST_HPP
