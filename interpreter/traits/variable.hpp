#pragma once

#include <type_traits>
#include "common.hpp"
#include "../variablereference.hpp"

template<class T>
constexpr bool is_var()
{
	return is_instantiation_of<T, VariableReference>();
}
