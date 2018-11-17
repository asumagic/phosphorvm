#pragma once

#include <unordered_map>
#include "variable.hpp"

struct Frame
{
	//std::unordered_map<std::int32_t, Variable> locals;

	//! Offset in the MainStack as the function gets called - i.e. one byte
	//! after the last pushed variable.
	std::size_t stack_offset = 0;

	//! Quantity of parameters passed to the function, as read from opcall.
	u16 argument_count = 0;
};
