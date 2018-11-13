#pragma once

#include <array>
#include <cstring>
#include <stdexcept>
#include <fmt/color.h>
#include <fmt/core.h>
#include "../util/errormanagement.hpp"
#include "../util/nametype.hpp"
#include "../config.hpp"
#include "traits.hpp"

class MainStack
{
	std::size_t _offset = 0;

public:
	std::array<char, max_stack_depth> raw;

	template<class T>
	T pop();

	template<class T>
	void push(const T& value);

	void push_raw(const void* source, std::size_t bytes);

	//! Skips 'count' bytes downward (i.e. pop). Negative values are accepted
	//! and skip the stack pointer upward.
	void skip(long count);

	std::size_t offset() const;
};

template<class T>
T MainStack::pop()
{
	if constexpr (numeric_type<T>::value)
	{
		_offset -= sizeof(T);

		T ret;
		std::memcpy(&ret, &raw[_offset], sizeof(T));

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
void MainStack::push(const T& value)
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

inline void MainStack::push_raw(const void* source, std::size_t bytes)
{
	std::memcpy(&raw[_offset], source, bytes);
	_offset += bytes;
}

inline void MainStack::skip(long count)
{
	_offset -= count;

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

inline std::size_t MainStack::offset() const
{
	return _offset;
}
