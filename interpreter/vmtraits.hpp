#pragma once

#include <type_traits>
#include "variable.hpp"

template<class... Ts>
using are_arithmetic = std::conjunction<std::is_arithmetic<Ts>...>;

template<class T, template<class> class TT>
struct is_instantiation_of : std::false_type {};

template<class T, template<class> class TT>
struct is_instantiation_of<TT<T>, TT> : std::true_type {};

template<class T>
struct vm_value_value_type { using value_type = T; };

template<template<class> class TT, class T>
struct vm_value_value_type<TT<T>> { using value_type = typename TT<T>::value_type; };

template<class T>
constexpr bool is_var()
{
	return is_instantiation_of<T, VariableReference>();
}

template<class T>
constexpr bool is_arithmetic_convertible()
{
	if constexpr (std::is_arithmetic_v<T>)
	{
		return true;
	}

	if constexpr (is_instantiation_of<T, VariableReference>::value)
	{
		return std::is_arithmetic_v<typename T::value_type>;
	}

	return false;
}

template<class... Ts>
constexpr bool is_arith_like()
{
	return (is_arithmetic_convertible<Ts>() && ...);
}