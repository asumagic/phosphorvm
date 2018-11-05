#pragma once

#include "contextstack.hpp"
#include "framestack.hpp"
#include "stack.hpp"
#include "variable.hpp"
#include "../config.hpp"
#include "../unpack/decode.hpp"
#include "../bytecode/types.hpp"
#include <tuple>
#include <unordered_map>
#include <stack>

// Seriously, don't bother looking at this unless you want to get rid of your
// sanity for good. This is the worst pile of garbage hacks since the creation
// of the Win32 API.

#define HELL(appended_type) \
	hell< \
		Left - 1, \
		F, \
		Ts..., \
		appended_type \
	>(f, new_array);

// std::visit helper
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class VM
{
	Form& form;

	MainStack stack;
	FrameStack frames;
	ContextStack context;

public:
	VM(Form& p_form) :
		form{p_form}
	{}

	// Worry not, this is not meant to be readable!
	//! Does some lame shit so it can call 'f' with all Left combinations of VM
	//! types so it can dispatch the 'types' passed to it.
	//! It builds up the 'Ts' parameter packs over subsequent calls.
	// No idea if it even compiles to remotely efficient code.
	// If it's inlined properly it should be fairly fast.
	// There's another layer of dumb crap so I can just abuse lambdas in
	// VM::execute. It passes a default-initialized list of the types rather
	// than invoking f<Ts...>(); because you can't do that with C++20 templated
	// lambdas.
	// TODO: deduce Left from std::array size
	// TODO: maybe split hell in two (because of ResolveVariableReferences)
	// TODO: make this usable to implement builtins sanely
	// TODO: this is probably awful in terms of compile times, improve that
	template<std::size_t Left, class F, class... Ts>
	void hell(F f, std::array<DataType, Left> types)
	{
		if constexpr (Left == 0)
		{
			(void)types; // inhibit unused parameter warning
			f(Ts{}...);
		}
		else
		{
			std::array<DataType, Left - 1> new_array;
			std::copy(types.begin() + 1, types.end(), new_array.begin());

			switch(types[0])
			{
			case DataType::f64: HELL(f64) break;
			case DataType::f32: HELL(f32) break;
			case DataType::i64: HELL(s64) break;
			case DataType::i32: HELL(s32) break;
			case DataType::i16: HELL(s16) break;
			case DataType::str: HELL(StringReference) break;
			case DataType::var: HELL(VariablePlaceholder) break;
			default: break;
			}
		}
	}

	void execute(const Script& script);
};
