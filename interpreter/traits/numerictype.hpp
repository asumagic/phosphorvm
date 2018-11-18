#pragma once

#include <type_traits>

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
