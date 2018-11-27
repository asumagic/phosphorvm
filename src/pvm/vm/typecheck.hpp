#pragma once

#include "pvm/bc/enums.hpp"
#include "pvm/vm/traits/datatype.hpp"
#include <stdexcept>

enum class TypeCheck
{
	Printable
};

template<class T>
constexpr bool type_check(TypeCheck check)
{
	constexpr auto data_type = data_type_for<T>::value;

	switch (check)
	{
	case TypeCheck::Printable:
		switch (data_type)
		{
		case DataType::b32:
		case DataType::f32:
		case DataType::f64:
		case DataType::i16:
		case DataType::i32:
		case DataType::i64:
			// TODO: str
			return true;

		default: return false;
		}
	}
}
