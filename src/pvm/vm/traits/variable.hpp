#pragma once

#include <type_traits>
#include "pvm/vm/traits/common.hpp"
#include "pvm/vm/variableoperand.hpp"

//! Returns true when T is a VariableOperand.
//! A value can be optionally passed to automatically deduce T.
template<class T>
constexpr bool is_var()
{
	return is_instantiation_of<T, VariableOperand>();
}

template<class T>
constexpr bool is_var(const T&)
{
	return is_var<T>();
}
