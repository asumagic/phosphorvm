#ifndef LIST_HPP
#define LIST_HPP

#include "chunk.hpp"
#include "../gmreader.hpp"
#include <vector>

template<class T>
struct List
{
	std::vector<T> elements;

	void debug_print() const
	{
		for (auto& element : elements)
		{
			element.debug_print();
		}
	}
};

template<class T>
struct ListChunk : Chunk, List<T> {};

template<class T>
void user_reader(List<T>& list, Reader& reader)
{
	s32 address_count = reader();

	for (s32 i = 0; i < address_count; ++i)
	{
		s32 address = reader();

		auto element_reader = reader;
		element_reader.seek(address);
		list.elements.push_back(element_reader());
	}
}

#endif // LIST_HPP
