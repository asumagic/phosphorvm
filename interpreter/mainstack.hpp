#pragma once

#include <array>
#include <cstring>
#include <stdexcept>
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
};

template<class T>
T MainStack::pop()
{
	if constexpr (std::is_fundamental_v<T>)
	{
		offset -= sizeof(T);

		T ret;
		std::memcpy(&ret, &raw[offset], sizeof(T));

		return ret;
	}

	throw std::runtime_error{
		"Unimplemented MainStack::pop for current type"
	};
}

template<class T>
void MainStack::push(const T& value)
{
	if constexpr (std::is_fundamental_v<T>)
	{
		std::memcpy(&raw[offset], &value, sizeof(T));
		offset += sizeof(T);
	}
	else
	{
		throw std::runtime_error{
			"Unimplemented MainStack::push for current type"
		};
	}
}
