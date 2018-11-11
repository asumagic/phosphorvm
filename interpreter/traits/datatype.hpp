#pragma once

#include <type_traits>
#include "../../bytecode/enums.hpp"
#include "../../bytecode/types.hpp"

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
