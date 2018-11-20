#pragma once

#include "contextstack.hpp"
#include "framestack.hpp"
#include "mainstack.hpp"
#include "traits/variable.hpp"
#include "variableoperand.hpp"
#include "../config.hpp"
#include "../unpack/chunk/form.hpp"
#include "../util/compilersupport.hpp"
#include "../util/errormanagement.hpp"
#include "../bytecode/types.hpp"

#define DISPATCH_NEXT(appended_type) \
	dispatcher< \
		Left - 1, \
		F, \
		Ts..., \
		appended_type \
	>(f, new_array);

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class VM
{
	const Form& form;

	MainStack stack;
	FrameStack frames;
	ContextStack contexts;

	std::size_t argument_offset(ArgId arg_id = 0) const;
	std::size_t local_offset(VarId var_id = 0) const;

public:
	VM(const Form& p_form);

	//! Calls a function 'f' with parameter types corresponding to the given
	//! 'types'. e.g. dispatcher(f, std::array{DataType::f32, DataType::f64})
	//! will call f(0.0f, 0.0);
	// TODO: make this usable to implement builtins sanely
	template<std::size_t Left, class F, class... Ts>
	void dispatcher(F f, std::array<DataType, Left> types);

	void read_special(SpecialVar var);

	template<class T>
	void push_stack_variable(const T& value, MainStackReader& reader);

	template<class T>
	void push_stack_variable(const T& value);

	template<class T>
	VariableOperand<T> read_variable_parameter(
		InstType inst_type = InstType::stack_top_or_global,
		VarId var_id = 0
	);

	template<class T>
	void write_variable(
		InstType inst_type,
		VarId var_id,
		VarType var_type,
		T value
	);

	template<class T>
	auto value(T& value);

	void print_stack_frame();

	void execute(const Script& script);
};

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

		switch(types.front())
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
void VM::push_stack_variable(const T& value, MainStackReader& reader)
{
	auto padding_bytes = sizeof(s64) - sizeof(T);

	reader.push(value);
	reader.skip(-padding_bytes);
	reader.push(data_type_for<T>::value);
}

template<class T>
FORCE_INLINE
void VM::push_stack_variable(const T& value)
{
	push_stack_variable(value, stack);
}

template<class T>
FORCE_INLINE
VariableOperand<T> VM::read_variable_parameter(InstType inst_type, VarId var_id)
{
	switch (inst_type)
	{
	case InstType::stack_top_or_global:
		stack.skip(sizeof(s64) - sizeof(T));
		return {stack.pop<T>()};

	default:
		maybe_unreachable("Unimplemented read_variable_reference for InstType");
	}
}

template<class T>
FORCE_INLINE
void VM::write_variable(
	InstType inst_type,
	VarId var_id,
	VarType var_type,
	T value
)
{
	switch (inst_type)
	{
	case InstType::stack_top_or_global:
		maybe_unreachable("Impossible to write_variable with this inst_type");

	case InstType::local: {
		MainStackReader reader{
			local_offset(form.vari.definitions[var_id].unknown),
			stack.raw
		};

		push_stack_variable(value, reader);
	} break;

	default:
		maybe_unreachable("Unimplemented write_variable for this inst_type");
	}
}

template<class T>
FORCE_INLINE
auto VM::value(T& value)
{
	if constexpr (is_var<T>())
	{
		return value.value;
	}
	else
	{
		return value;
	}
}
