#include "vm.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>
#include <type_traits>

// Please don't scroll down.

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

// I tried to warn you.

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

	auto binop = [&](auto handler) {
		auto [t1, t2] = decode_type_pair();
		hell<2>([&](auto a, auto b) {
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

	// TODO: copypasted garbage from disasm.cpp, merge somehow?
	auto read_block_operand = [&](auto& into) {
		std::memcpy(&into, block, sizeof(decltype(into)));
		block += sizeof(decltype(into)) / sizeof(Block);
		return into;
	};

	// TODO: further copypasted garbage
	auto decode_and_push_value = [&](auto placeholder) {
		if constexpr (std::is_fundamental_v<decltype(placeholder)>)
		{
			read_block_operand(placeholder);
			push(placeholder);
		}
		else
		{
			throw std::runtime_error{"Unhandled push value type"};
		}
	};

	while (block != end_block)
	{
		auto opcode = *block >> 24;

		switch (Instr(opcode))
		{
		case Instr::opmul:
			binop([&]<class A, class B>(A a, B b) {
				if constexpr (is_arith_like<A, B>())
				{
					push(a * b);
				}
				else if constexpr (std::is_same_v<A, StringReference> && is_arith_like<B>())
				{
					// TODO: repeat string
				}
				else { fail_impossible(); }
			});
			break;

		BINOP_ARITH(opdiv, /)
		// case Instr::oprem: // TODO
		// case Instr::opmod: // TODO
		BINOP_ARITH(opadd, +)
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
		case Instr::oppushcst: {
			auto [t1, t2] = decode_type_pair();
			hell<1>(decode_and_push_value, {t1});
		} break;
		// case Instr::oppushloc: // TODO
		// case Instr::oppushglb: // TODO
		// case Instr::oppushvar: // TODO
		// case Instr::opcall: // TODO
		// case Instr::opbreak: // TODO

		default:
			fmt::print(fmt::color::red, "Unhandled op {:02x}\n", opcode);
			return;
		}

		++block;
	}
}
