#pragma once

#include "contextstack.hpp"
#include "framestack.hpp"
#include "mainstack.hpp"
#include "../config.hpp"
#include "../unpack/chunk/form.hpp"
#include "../util/compilersupport.hpp"
#include "../util/errormanagement.hpp"
#include "../bytecode/types.hpp"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct VM
{
	const Form& form;

	MainStack stack;
	FrameStack frames;
	ContextStack contexts;

	VM(const Form& p_form) :
		form{p_form}
	{
		if constexpr (check(debug::vm_debug_stack))
		{
			std::fill(stack.raw.begin(), stack.raw.end(), 0xAB);
		}
	}

	//! Calls a function 'f' with parameter types corresponding to the given
	//! 'types'. e.g. dispatcher(f, std::array{DataType::f32, DataType::f64})
	//! will call f(0.0f, 0.0);
	// TODO: make this usable to implement builtins sanely
	// TODO: this is probably awful in terms of compile times, improve that
	template<std::size_t Left, class F, class... Ts>
	void dispatcher(F f, std::array<DataType, Left> types);

	void read_special(SpecialVar var);

	template<class T>
	void push_stack_variable(const T& value);

	DataType pop_variable_var_type(InstType inst_type);

	template<class T>
	auto value(T& value);

	void print_stack_frame();

	void execute(const Script& script);
};

#include "vminline.hpp"
