#pragma once

#include <type_traits>

template<class T>
auto enum_value(T value)
{
	return static_cast<std::underlying_type_t<T>>(value);
}
