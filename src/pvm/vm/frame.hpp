#pragma once

#include <unordered_map>
#include "pvm/vm/variable.hpp"

struct Frame
{
	//std::unordered_map<std::int32_t, Variable> locals;

	//! Offset in the MainStack as the function gets called - i.e. one byte
	//! after the last pushed variable.
	std::size_t stack_offset = 0;

	//! Quantity of parameters passed to the function, as read from opcall.
	u16 argument_count = 0;

	[[nodiscard]] std::size_t argument_offset(ArgId arg_id = 0) const;
	[[nodiscard]] std::size_t local_offset(VarId var_id = 0) const;
};

inline std::size_t Frame::argument_offset(ArgId arg_id) const
{
	return stack_offset + arg_id * Variable::stack_variable_size;
}

inline std::size_t Frame::local_offset(VarId var_id) const
{
	return argument_offset(argument_count) + var_id * Variable::stack_variable_size;
}
