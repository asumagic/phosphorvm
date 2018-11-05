#pragma once

#include <array>
#include "../config.hpp"

class MainStack
{
	std::size_t offset = 0;

public:
	std::array<char, max_stack_depth> raw;

	template<class T>
	T peek() const;

	template<class T>
	T pop() const;

	template<class T>
	void push() const;
};
