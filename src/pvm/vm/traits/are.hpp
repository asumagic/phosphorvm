#pragma once

#include <type_traits>

//! Returns true when all values of the parameter pack match the TT trait.
//! Values can be optionally passed to automatically deduce Ts.
//! Example usages:
//! - are<std::is_arithmetic, A, B, C>()
//! - are<std::is_arithmetic>(a, b, c);
template<template<class> class TT, class... Ts>
[[nodiscard]] constexpr bool are()
{
	return (TT<Ts>::value && ...);
}

template<template<class> class TT, class... Ts>
[[nodiscard]] constexpr bool are(const Ts&...)
{
	return are<TT, Ts...>();
}
