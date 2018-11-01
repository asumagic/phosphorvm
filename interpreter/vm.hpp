#ifndef VM_HPP
#define VM_HPP

#include "../config.hpp"
#include "../unpack/decode.hpp"
#include "../bytecode/types.hpp"
#include <tuple>

#define HELL(appended_type) \
	hell<Left - 1, F, Ts..., appended_type>(f, new_array);

// std::visit helper
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct VariableInfo
{
	InstType inst_type;
	DataType data_type;
};

template<class T>
struct VariableReference
{
	using value_type = T;
};

struct StringReference
{

};

class VM
{
public:
	Form& _form;

	std::vector<char> stack;

	template<class T>
	T pop_raw();

	template<class T>
	T pop()
	{
		return pop_raw<T>();
	}

	template<class T>
	void push_raw(const T& value)
	{
		stack.resize(stack.size() + sizeof(T));
		std::memcpy(stack.data() + stack.size() - sizeof(T), &value, sizeof(T));
	}

	template<class T>
	void push(const T& value)
	{
		fmt::print("Push value {}\n", value);
		push(value);
	}

public:
	VM(Form& form) :
		_form{form}
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
			case DataType::var: HELL(VariableInfo) break;
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
