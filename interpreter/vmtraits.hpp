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

// Defines type to the underlying type of an enum if T is an enum; T otherwise.
template<class T>
class numeric_type
{
	static constexpr auto type_deducer()
	{
		if constexpr (std::is_arithmetic_v<T>)
		{
			return T{};
		}
		else if constexpr (std::is_enum_v<T>)
		{
			return std::underlying_type_t<T>{};
		}
		else
		{
			return void();
		}
	}

public:
	using type = decltype(type_deducer());
	static constexpr bool value = !std::is_void_v<type>;
};

template<class T>
class data_type_for
{
	static constexpr auto value_deducer() noexcept
	{
		if constexpr (std::is_same_v<T, s16>) { return DataType::i16; }
		if constexpr (std::is_same_v<T, s32>) { return DataType::i32; }
		if constexpr (std::is_same_v<T, s64>) { return DataType::i64; }
		if constexpr (std::is_same_v<T, f32>) { return DataType::f32; }
		if constexpr (std::is_same_v<T, f64>) { return DataType::f64; }
		if constexpr (std::is_same_v<T, StringReference>) { return DataType::str; }
		if constexpr (is_var<T>() || std::is_same_v<T, VariablePlaceholder>) { return DataType::var; }
	}

public:
	static constexpr DataType value = value_deducer();
};

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
