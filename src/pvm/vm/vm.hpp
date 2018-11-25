#pragma once

#include "pvm/bc/enums.hpp"
#include "pvm/bc/types.hpp"
#include "pvm/config.hpp"
#include "pvm/vm/contextstack.hpp"
#include "pvm/vm/framestack.hpp"
#include "pvm/vm/mainstack.hpp"
#include "pvm/vm/traits/variable.hpp"
#include "pvm/vm/variableoperand.hpp"
#include "pvm/vm/vmstate.hpp"
#include "pvm/std/everything.hpp"
#include "pvm/unpack/chunk/form.hpp"
#include "pvm/util/compilersupport.hpp"
#include "pvm/util/errormanagement.hpp"

#define DISPATCH_NEXT(appended_type) \
	dispatcher< \
		Left - 1, \
		F, \
		Ts..., \
		appended_type \
	>(f, new_array);

class VM
{
	const Form& form;

	MainStack stack;
	FrameStack frames;
	ContextStack contexts;

	Bindings bindings;

	VarId local_id_from_reference(u32 reference) const;

public:
	VM(const Form& p_form);

	//! Calls a function 'f' with parameter types corresponding to the given
	//! 'types'. e.g. dispatcher(f, std::array{DataType::f32, DataType::f64})
	//! will call f(0.0f, 0.0);
	// TODO: make this usable to implement builtins sanely
	template<std::size_t Left, class F, class... Ts>
	void dispatcher(F f, std::array<DataType, Left> types);

	//! Calls a handler providing it a value of the given type popped from
	//! the stack.
	//! When encountering variables, will provide a VariableReference<T>
	//! with T being the variable type as read on the stack.
	template<class T>
	auto pop_dispatch(VMState& state, T handler, DataType type);

	//! Executes 'handler' as an instruction that pops two parameters.
	template<class T>
	auto op_pop2(VMState& state, T handler);

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

	void call(const FunctionDefinition& func, std::size_t argument_count = 0);

	void execute(const Script& script);
};

inline VM::VM(const Form& p_form) :
	form{p_form},
	bindings{p_form.func}
{
	bind_everything(bindings);

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
auto VM::pop_dispatch(VMState& state, T handler, DataType type)
{
	if (type == DataType::var)
	{
		return dispatcher([&](auto v) {
			return handler(read_variable_parameter<decltype(v)>());
		}, std::array{stack.pop<DataType>()});
	}

	return dispatcher([&](auto v) {
		return handler(stack.pop<decltype(v)>());
	}, std::array{type});
};

template<class T>
FORCE_INLINE
auto VM::op_pop2(VMState& state, T handler)
{
	// Parameters are correctly reversed here
	return pop_dispatch(state, [&](auto b) {
		return pop_dispatch(state, [&](auto a) {
			if constexpr (check(debug::vm_verbose_instructions))
			{
				fmt::print(
					fmt::color::yellow_green,
					"    f(pop<{}>(), pop<{}>())\n",
					type_name<decltype(a)>(),
					type_name<decltype(b)>()
				);
			}

			return handler(a, b);
		}, state.t2);
	}, state.t1);
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
		auto local_offset = frames.top().local_offset(local_id_from_reference(var_id));
		MainStackReader reader = stack.temporary_reader(local_offset);

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
