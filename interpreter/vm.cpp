#include "vm.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>
#include <type_traits>

#define BINOP_ARITH(name, op) case Instr:: name : binop_arithmetic([&](auto a, auto b) { return a op b; }); break;

void fail_impossible()
{
	if constexpr (debug_mode)
	{
		throw std::logic_error{"VM state should never be reached"};
	}

	__builtin_unreachable();
}

template<class... Ts>
using are_arithmetic = std::conjunction<std::is_fundamental<Ts>...>;

template<class T, template<class> class TT>
struct is_instantiation_of : std::false_type {};

template<class T, template<class> class TT>
struct is_instantiation_of<TT<T>, TT> : std::true_type {};

template<class T>
struct vm_value_value_type { using value_type = T; };

template<template<class> class TT, class T>
struct vm_value_value_type<TT<T>> { using value_type = typename TT<T>::value_type; };

template<class T>
constexpr bool is_arithmetic_convertible()
{
	if constexpr (std::is_fundamental_v<T>)
	{
		return true;
	}

	if constexpr (is_instantiation_of<T, VariableReference>::value)
	{
		return std::is_fundamental_v<typename T::value_type>;
	}

	return false;
}

template<class... Ts>
constexpr bool is_arith_like()
{
	return (is_arithmetic_convertible<Ts>() && ...);
}

void VM::execute(const Script& script)
{
	const Block* block = script.data.data();
	const Block* end_block = block + script.data.size();

	auto decode_type_pair = [&] {
		return std::pair{
			DataType((*block >> 16) & 0xF),
			DataType((*block >> 12) & 0xF)
		};
	};

	auto decode_opcode = [&] {
		return *block >> 24;
	};

	while (block != end_block)
	{
		auto opcode = decode_opcode();
		auto [t1, t2] = decode_type_pair();

		fmt::print(
			"Execution trace: at ${:08x} opcode ${:02x}. stack data: {:02x}\n",
			std::distance(script.data.data(), block),
			opcode,
			fmt::join(stack, " ")
		);

		auto binop = [&](auto handler) {
			hell<true, 2>([&](auto a, auto b) {
				handler(pop<decltype(a)>(), pop<decltype(b)>());
			}, {t1, t2});
		};

		auto binop_arithmetic = [&](auto handler) {
			binop([&]<class A, class B>(A a, B b) {
				if constexpr (is_arith_like<A, B>())
				{
					push(handler(a, b));
				}
			});
		};

		auto op_push_var = [&] {
			auto reference = *(++block);
			auto inst_type = InstType(reference >> 24);

			// Override instance type for certain opcodes as an optimization
			// TODO: make this by passing an argument to the lambda somehow
			switch (Instr(opcode))
			{
			case Instr::oppushloc:
				inst_type = InstType::local;
				break;

			case Instr::oppushglb:
				inst_type = InstType::global;
				break;

			// This case is somewhat different. I am not certain whether it
			// results into an actual improvement.
			case Instr::oppushvar: {
				if (inst_type == InstType::global
				 || inst_type == InstType::local)
				{
					fail_impossible();
				}
			} break;

			default: break;
			}

			// Using hell to dispatch
			hell<true, 1>([&](auto v) {

			}, {DataType::var});
		};

		switch (Instr(opcode))
		{
		// TODO: check if multiplying strings with int is actually possible
		BINOP_ARITH(opmul, *)
		BINOP_ARITH(opdiv, /)
		// case Instr::oprem: // TODO
		// case Instr::opmod: // TODO
		BINOP_ARITH(opadd, +) // TODO: you can add strings together, too
		BINOP_ARITH(opsub, -)
		//BINOP_ARITH(opand, &)
		//BINOP_ARITH(opor,  |)
		//BINOP_ARITH(opxor, ^)
		// case Instr::opneg: // TODO
		// case Instr::opnot: // TODO
		// case Instr::opshl: // TODO
		// case Instr::opshr: // TODO
		// case Instr::opcmp: // TODO
		// case Instr::oppop: // TODO
		// case Instr::oppushi16: // TODO
		// case Instr::opdup: // TODO
		// case Instr::opret: // TODO
		// case Instr::opexit: // TODO
		// case Instr::oppopz: // TODO
		// case Instr::opb: // TODO
		// case Instr::opbt: // TODO
		// case Instr::opbf: // TODO
		// case Instr::oppushenv: // TODO
		// case Instr::oppopenv: // TODO

		//case Instr::oppushcst: break;

		case Instr::oppushloc:
		case Instr::oppushglb:
		case Instr::oppushvar:
			op_push_var();
			break;

		// case Instr::opcall: // TODO
		// case Instr::opbreak: // TODO

		default:
			fmt::print(fmt::color::red, "Unhandled op ${:02x}\n", opcode);
			return;
		}

		++block;
	}
}
