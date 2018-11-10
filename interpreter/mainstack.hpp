#pragma once

#include <array>
#include <cstring>
#include <stdexcept>
#include <fmt/color.h>
#include <fmt/core.h>
#include "../util/nametype.hpp"
#include "../config.hpp"
#include "vmtraits.hpp"

struct MainStack
{
	std::size_t offset = 0;
	std::array<char, max_stack_depth> raw;

	template<class T>
	T pop();

	template<class T>
	void push(const T& value);

	void push_raw(const void* source, std::size_t bytes);

	void skip(std::size_t count);
};

template<class T>
T MainStack::pop()
{
	if constexpr (numeric_type<T>::value)
	{
		offset -= sizeof(T);

		T ret;
		std::memcpy(&ret, &raw[offset], sizeof(T));

		if constexpr (debug_mode)
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

	throw std::runtime_error{
		"Unimplemented MainStack::pop for current type"
	};
}

template<class T>
void MainStack::push(const T& value)
{
	if constexpr (numeric_type<T>::value)
	{
		if constexpr (debug_mode)
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
		throw std::runtime_error{
			"Unimplemented MainStack::push for current type"
		};
	}
}

inline void MainStack::push_raw(const void* source, std::size_t bytes)
{
	std::memcpy(&raw[offset], source, bytes);
	offset += bytes;
}

inline void MainStack::skip(std::size_t count)
{
	offset -= count;
}
