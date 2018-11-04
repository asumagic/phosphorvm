#ifndef VM_HPP
#define VM_HPP

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

//! Type used as a placeholder in hell so instructions can detect variables
//! through type information. This may be passed to the 'f' parameter when
//! called by VM:hell only when ResolveVariableReferences is false.
// It doesn't even have InstType information because we may have to peek the
// stack in order to read that value, which may be the unwanted behavior.
// pop<VariablePlaceholder> should successfully skip the value, however.
struct VariablePlaceholder {};

struct StringReference
{

};

struct Variable
{
	DataType type;

	union TypeUnion
	{
		f64 vf64;
		f32 vf32;
		s64 vs64;
		s32 vs32;
		s16 vs16;
		bool vb32;
		StringReference vstr;
	};
};

//! Typed variable reference, which can be obtained as a parameter in 'f' when
//! calling VM::hell, assuming ResolveVariableReferences is true.
template<class T>
struct VariableReference
{
	using value_type = T;
	InstType inst_type;
	VM& vm;

	T pop()
	{
		if (inst_type == InstType::stack_top_or_global)
		{
			return {};
		}

		return {};
	}
};

struct Instance
{
	std::unordered_map<std::int32_t, Variable> variables;
};

struct LocalFrame
{
	std::unordered_map<std::int32_t, Variable> locals;
};

class VM
{
	Form& _form;

	std::vector<char> stack;

	template<class T>
	T pop_raw();

	template<class T>
	void push_raw(const T& value)
	{
		stack.resize(stack.size() + sizeof(T));
		std::memcpy(stack.data() + stack.size() - sizeof(T), &value, sizeof(T));
	}

	std::unordered_map<std::int32_t, Variable> globals;
	std::unordered_map<InstId, Instance> instances;
	std::stack<LocalFrame> frame_stack;

public:
	VM(Form& form) :
		_form{form}
	{}

	//! Pop a value from the stack, if applicable. When fetching a value from
	//! outside the stack, the data will not be popped, just read.
	//! For instance, with VariableReference, if inst_type == InstType::Global
	//! then pop() will only read the variable.
	//! If inst_type == stack_top_or_global then the variable *will* be popped
	//! from the stack.
	template<class T>
	T pop()
	{
		return pop_raw<T>();
	}

	template<class T>
	void push(const T& value)
	{
		fmt::print("Push value {}\n", value);
		push(value);
	}

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

template<class T>
T VM::pop_raw()
{
	// TODO: when debug_mode is set, do sanity checks
	char* data = stack.data() + stack.size() - sizeof(T);

	T ret;
	std::memcpy(&ret, data, sizeof(T));
	return ret;
}

#endif // VM_HPP
