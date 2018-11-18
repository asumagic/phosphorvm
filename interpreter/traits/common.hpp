#pragma once

#include <type_traits>

//! Returns true when all values of the parameter pack match the TT trait.
//! Values can be optionally passed to automatically deduce Ts.
//! Example usages:
//! - are<std::is_arithmetic, A, B, C>()
//! - are<std::is_arithmetic>(a, b, c);
template<template<class> class TT, class... Ts>
constexpr bool are()
{
	return (TT<Ts>::value && ...);
}

template<template<class> class TT, class... Ts>
constexpr bool are(const Ts&...)
{
	return (TT<Ts>::value && ...);
}

//! Inherits from std::true_type when TT is a template instanciation of T.
template<class T, template<class> class TT>
struct is_instantiation_of : std::false_type {};

template<class T, template<class> class TT>
struct is_instantiation_of<TT<T>, TT> : std::true_type {};

//! Defines type to the underlying type of an enum if T is an enum; T otherwise.
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
