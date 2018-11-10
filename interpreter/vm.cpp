#include "vm.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>
#include <type_traits>
#include "vmtraits.hpp"

#define BINOP_ARITH(name, op) case Instr::name : \
	binop_arithmetic([&](auto a, auto b) { \
		return vm_value(a) op vm_value(b); \
	}); \
	break;

void fail_impossible()
{
	if constexpr (debug_mode)
	{
		throw std::logic_error{"VM state should never be reached"};
	}

	__builtin_unreachable();
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
			fmt::join(std::vector(&stack.raw[0], &stack.raw[stack.offset]), " ")
		);

		auto vm_value = [&](auto v) {
			if constexpr (is_var<decltype(v)>())
			{
				return pop_variable(v);
			}
			else
			{
				// NB: This HAS to be in an else branch rather than just
				// outside of the 'if' because otherwise the return type cannot
				// be deduced because of the two returns. Eat that, clang-tidy!
				return v;
			}
		};

		auto pop_parameter = [&](auto handler, DataType type) {
			if (type == DataType::var)
			{
				auto type = stack.pop<InstType>();
				return dispatcher([&](auto v) {
					handler(stack.pop<VariableReference<decltype(v)>>());
				}, std::array{pop_variable_var_type(type)});
			}

			return dispatcher(handler, std::array{type});
		};

		auto stack_dispatch_2 = [&, t1=t1, t2=t2](auto handler) {
			pop_parameter([&](auto a) {
				pop_parameter([&](auto b) {
					handler(a, b);
				}, t2);
			}, t1);
		};

		auto binop_arithmetic = [&](auto handler) {
			stack_dispatch_2([&](auto a, auto b) {
				if constexpr (is_arith_like<decltype(a), decltype(b)>())
				{
					stack.push(handler(a, b));
				}
			});
		};

		auto branch = [&] {
			auto offset = *block & 0xFFFFFF;
			// TODO: make this nicer somehow? skipping block++ on the end
			block += offset;
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

		case Instr::opcmp: {
			auto func = CompFunc((*block >> 8) & 0xFF);
			stack_dispatch_2([&](auto a, auto b) {
				stack.push<bool>([](auto func, auto a, auto b) -> bool {
					if constexpr (is_arith_like<decltype(a), decltype(b)>())
					{
						switch (func)
						{
						case CompFunc::lt:  return a <  b;
						case CompFunc::lte: return a <= b;
						case CompFunc::eq:  return a == b;
						case CompFunc::neq: return a != b;
						case CompFunc::gte: return a >= b;
						case CompFunc::gt:  return a >  b;
						default: fail_impossible();
						}
					}

					fail_impossible();
				}(func, vm_value(a), vm_value(b)));
			});
		} break;

		// case Instr::oppop: // TODO
		// case Instr::oppushi16: // TODO
		// case Instr::opdup: // TODO
		// case Instr::opret: // TODO
		// case Instr::opexit: // TODO
		// case Instr::oppopz: // TODO

		case Instr::opb: branch();
		case Instr::opbt: if (stack.pop<bool>()) { branch(); }
		case Instr::opbf: if (stack.pop<bool>()) { branch(); }

		// case Instr::oppushenv: // TODO
		// case Instr::oppopenv: // TODO

		//case Instr::oppushcst: break;

		//case Instr::oppushloc:
		//case Instr::oppushglb:

		case Instr::oppushspc:
			push_special(SpecialVar(*(++block) & 0x00FFFFFF));
			break;

		case Instr::oppushi16:
			stack.push<s16>(*block & 0xFFFF);
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

void VM::push_special(SpecialVar var)
{
	// argumentn
	if (unsigned(var) != 0 && unsigned(var) <= 17)
	{
		stack.push_raw(
			&stack.raw[frames.top().stack_offset - Variable::stack_variable_size * unsigned(var)],
			Variable::stack_variable_size
		);

		return;
	}
}

DataType VM::pop_variable_var_type(InstType inst_type)
{
	switch (inst_type)
	{
	case InstType::stack_top_or_global:
		return stack.pop<DataType>();

	default:
		throw std::runtime_error{"Unhandled variable type"};
	}
}
