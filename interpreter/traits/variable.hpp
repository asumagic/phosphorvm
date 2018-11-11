#pragma once

#include <type_traits>
#include "common.hpp"
#include "../variable.hpp"

template<class T>
constexpr bool is_var()
{
	return is_instantiation_of<T, VariableReference>();
}

//! Returns true when T is arithmetic or if it is a VM variable which can be
//! converted to an arithmetic value.
template<class T>
constexpr bool is_arithmetic_convertible()
{
	if constexpr (std::is_arithmetic_v<T>)
	{
		return true;
	}

	if constexpr (is_var<T>())
	{
		return std::is_arithmetic_v<typename T::value_type>;
	}

	return false;
}

template<class... Ts>
constexpr bool are_arithmetic_convertible()
{
	return (is_arithmetic_convertible<Ts>() && ...);
}
