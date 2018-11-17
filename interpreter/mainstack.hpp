#pragma once

#include <array>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <fmt/color.h>
#include <fmt/core.h>
#include "../util/errormanagement.hpp"
#include "../util/nametype.hpp"
#include "../config.hpp"
#include "traits/datatype.hpp"

struct MainStackReader
{
	std::size_t offset;
	std::array<char, max_stack_depth>& raw_ref;

	template<class T>
	T pop();

	template<class T>
	void push(const T& value);

	void push_raw(const void* source, std::size_t bytes);

	//! Skips 'count' bytes downward (i.e. pop). Negative values are accepted
	//! and skip the stack pointer upward.
	void skip(long count);
};

class MainStack : public MainStackReader
{
public:
	MainStack();

	std::array<char, max_stack_depth> raw;

	void seek(MainStackReader reader);

	MainStackReader temporary_reader();
	MainStackReader temporary_reader(std::size_t _offset);
};

inline MainStack::MainStack() :
	MainStackReader{0, raw}
{}

inline void MainStack::seek(MainStackReader reader)
{
	offset = reader.offset;
}

inline MainStackReader MainStack::temporary_reader()
{
	return {*this};
}

inline MainStackReader MainStack::temporary_reader(std::size_t offset)
{
	return {offset, raw};
}

template<class T>
T MainStackReader::pop()
{
	if constexpr (numeric_type<T>::value)
	{
		offset -= sizeof(T);

		T ret;
		std::memcpy(&ret, &raw_ref[offset], sizeof(T));

		if constexpr (check(debug::vm_verbose_stack))
		{
			fmt::print(
				fmt::color::dark_magenta,
				">>> Popping {:20} <T = {:15}, sizeof(T) = {}>\n",
				typename numeric_type<T>::type(ret),
				type_name<T>(),
				sizeof(T)
			);
		}

		return ret;
	}

	maybe_unreachable("Unimplemented MainStack::push for current type");
}

template<class T>
void MainStackReader::push(const T& value)
{
	if constexpr (numeric_type<T>::value)
	{
		if constexpr (check(debug::vm_verbose_stack))
		{
			fmt::print(
				fmt::color::maroon,
				"<<< Pushing {:20} <T = {:15}, sizeof(T) = {}>\n",
				typename numeric_type<T>::type(value),
				type_name<T>(),
				sizeof(T)
			);
		}

		push_raw(&value, sizeof(T));
	}
	else
	{
		maybe_unreachable("Unimplemented MainStack::push for current type");
	}
}

inline void MainStackReader::push_raw(const void* source, std::size_t bytes)
{
	std::memcpy(&raw_ref[offset], source, bytes);
	offset += bytes;
}

inline void MainStackReader::skip(long count)
{
	offset -= count;

	if constexpr (check(debug::vm_verbose_stack))
	{
		if (count > 0)
		{
			fmt::print(
				fmt::color::dark_magenta,
				">>> Skipping {:4} bytes downward\n",
				count
			);
		}
		else if (count < 0)
		{
			fmt::print(
				fmt::color::maroon,
				"<<< Skipping {:4} bytes upward\n",
				-count
			);
		}
	}
}
