#pragma once
#include "vm.hpp"
#include "traits/variable.hpp"
#include "variable.hpp"

#define DISPATCH_NEXT(appended_type) \
	dispatcher< \
		Left - 1, \
		F, \
		Ts..., \
		appended_type \
	>(f, new_array);

inline VM::VM(const Form& p_form) :
	form{p_form}
{
	if constexpr (check(debug::vm_debug_stack))
	{
		std::fill(stack.raw.begin(), stack.raw.end(), 0xAB);
	}
}

template<std::size_t Left, class F, class... Ts>
FORCE_INLINE
void VM::dispatcher(F f, [[maybe_unused]] std::array<DataType, Left> types)
{
	if constexpr (Left == 0)
	{
		f(Ts{}...);
	}
	else
	{
		std::array<DataType, Left - 1> new_array;
		std::copy(types.begin() + 1, types.end(), new_array.begin());

		switch(types[0])
		{
		case DataType::f64: DISPATCH_NEXT(f64) break;
		case DataType::f32: DISPATCH_NEXT(f32) break;
		case DataType::i64: DISPATCH_NEXT(s64) break;
		case DataType::i32: DISPATCH_NEXT(s32) break;
		case DataType::i16: DISPATCH_NEXT(s16) break;
		case DataType::str: DISPATCH_NEXT(StringReference) break;
		case DataType::var: DISPATCH_NEXT(VariablePlaceholder) break;
		default: maybe_unreachable();
		}
	}
}

template<class T>
FORCE_INLINE
void VM::push_stack_variable(const T& value)
{
	auto padding_bytes = sizeof(s64) - sizeof(T);

	stack.push(value);
	stack.skip(-padding_bytes);
	stack.push(data_type_for<T>::value);
}

template<class T>
FORCE_INLINE
auto VM::value(T& value)
{
	if constexpr (is_var<T>())
	{
		return value.read(*this);
	}
	else
	{
		// NB: This HAS to be in an else branch rather than just
		// outside of the 'if' because otherwise the return type cannot
		// be deduced because of the two returns. Eat that, clang-tidy!
		return value;
	}
}
