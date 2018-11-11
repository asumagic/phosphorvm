#pragma once

#include "contextstack.hpp"
#include "framestack.hpp"
#include "mainstack.hpp"
#include "variable.hpp"
#include "../config.hpp"
#include "../unpack/chunk/form.hpp"
#include "../util/compilersupport.hpp"
#include "../bytecode/types.hpp"
#include <tuple>
#include <unordered_map>
#include <stack>

// Seriously, don't bother looking at this unless you want to get rid of your
// sanity for good. This is the worst pile of garbage hacks since the creation
// of the Win32 API.

#define DISPATCH_NEXT(appended_type) \
	dispatcher< \
		Left - 1, \
		F, \
		Ts..., \
		appended_type \
	>(f, new_array);

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

//! Call to designate a normally unreachable spot.
//! With debug_mode set, throws an exception. Otherwise, uses
//! __builtin_unreachble (effectively becoming an optimization).
[[noreturn]]
void fail_impossible();

class VM
{
	Form& form;

	MainStack stack;
	FrameStack frames;
	ContextStack contexts;

public:
	VM(Form& p_form) :
		form{p_form}
	{}

	//! Calls a function 'f' with parameter types corresponding to the given
	//! 'types'. e.g. dispatcher(f, std::array{DataType::f32, DataType::f64})
	//! will call f(0.0f, 0.0);
	// TODO: make this usable to implement builtins sanely
	// TODO: this is probably awful in terms of compile times, improve that
	template<std::size_t Left, class F, class... Ts>
	void dispatcher(F f, std::array<DataType, Left> types);

	void push_special(SpecialVar var);

	template<class T>
	void push_stack_variable(const T& value);

	DataType pop_variable_var_type(InstType inst_type);

	template<class T>
	T pop_variable(VariableReference<T>& variable);

	template<class T>
	auto value(T& value);

	void print_stack_frame();

	void execute(const Script& script);
};

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
		default: break;
		}
	}
}

template<class T>
FORCE_INLINE
void VM::push_stack_variable(const T& value)
{
	auto padding_bytes = sizeof(s64) - sizeof(T);

	stack.push(value);

	// TODO: check if this is fast enough. might be ok to
	// leave padding uninitialized?
	for (std::size_t i = 0; i < padding_bytes; ++i)
	{
		stack.push<u8>(0);
	}

	stack.push(data_type_for<T>::value);
	stack.push(InstType::stack_top_or_global);
}

template<class T>
FORCE_INLINE
T VM::pop_variable(VariableReference<T>& variable)
{
	if (!variable.cached_value)
	{
		switch (variable.inst_type)
		{
		case InstType::stack_top_or_global:
			// At this time the only part of the stack variable that is left is
			// the actual value, *including padding*, which we have to care about!
			stack.skip(sizeof(s64) - sizeof(T));
			variable.cached_value = stack.pop<T>();
			break;

		default:
			throw std::runtime_error{"Unimplemented pop_variable for T"};
		}
	}

	return variable.cached_value.value();
}

template<class T>
FORCE_INLINE
auto VM::value(T& value)
{
	if constexpr (is_var<T>())
	{
		return pop_variable(value);
	}
	else
	{
		// NB: This HAS to be in an else branch rather than just
		// outside of the 'if' because otherwise the return type cannot
		// be deduced because of the two returns. Eat that, clang-tidy!
		return value;
	}
}
