#include "vm.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>
#include <type_traits>
#include "vmtraits.hpp"
#include "../util/nametype.hpp"

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
	if constexpr (debug_mode)
	{
		if (frames.offset > 32)
		{
			fmt::print("Reached debug max call depth size\n");
			exit(0);
		}

		fmt::print(
			fmt::color::red,
			"\nExecuting function '{}' (call depth {})",
			script.name,
			frames.offset + 1
		);
	}

	const Block* block = script.data.data();
	const Block* end_block = block + script.data.size();

	while (block != end_block)
	{
		auto opcode = *block >> 24;
		auto t1 = DataType((*block >> 16) & 0xF);
		auto t2 = DataType((*block >> 20) & 0xF);

		if constexpr (debug_mode)
		{
			Frame& frame = frames.top();

			fmt::print(
				fmt::color::orange,
				"\nExecution trace: ${:08x}: ${:02x}.",
				std::distance(script.data.data(), block),
				opcode
			);

			fmt::print(
				fmt::color::gray,
				" Stack frame data ({:5} bytes): {:02x}\n",
				stack.offset - frame.stack_offset,
				fmt::join(std::vector(&stack.raw[frame.stack_offset], &stack.raw[stack.offset]), " ")
			);
		}

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
				auto inst_type = stack.pop<InstType>();
				auto data_type = pop_variable_var_type(inst_type);
				return dispatcher([&](auto v) {
					VariableReference<decltype(v)> var{inst_type, data_type};
					return handler(pop_variable(var));
				}, std::array{data_type});
			}

			return dispatcher([&](auto v) {
				handler(stack.pop<decltype(v)>());
			}, std::array{type});
		};

		auto stack_dispatch_2 = [&](auto handler) {
			pop_parameter([&](auto a) {
				pop_parameter([&](auto b) {
					if constexpr (debug_mode)
					{
						fmt::print(
							fmt::color::yellow_green,
							"    Types for popping instr: a = {} (from {}), b = {} (from {})\n",
							type_name<decltype(a)>(),
							unsigned(t1),
							type_name<decltype(b)>(),
							unsigned(t2)
						);
					}

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
			block += offset - 1;
		};

		switch (Instr(opcode))
		{
		case Instr::opconv:
			pop_parameter([&](auto src) {
				dispatcher([&](auto dst) {
					if constexpr (std::is_same_v<decltype(dst), VariablePlaceholder>)
					{
						auto padding_bytes = sizeof(s64) - sizeof(src);

						// TODO: check if this is fast enough. might be ok to
						// leave padding uninitialized?
						for (std::size_t i = 0; i < padding_bytes; ++i)
						{
							stack.push<u8>(0);
						}

						stack.push(t1);
						stack.push(InstType::stack_top_or_global);
					}
					else if constexpr (std::is_arithmetic_v<decltype(dst)>
									&& is_arithmetic_convertible<decltype(src)>())
					{
						stack.push<decltype(dst)>(vm_value(src));
					}
					else
					{
						throw std::runtime_error{"Unimplemented conversion types"};
					}
				}, std::array{t2});
			}, t1);
			break;

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
					(void)func;

					if constexpr (are_arithmetic<decltype(a), decltype(b)>())
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

		case Instr::opret: {
			std::move(
				stack.raw.begin() + stack.offset - Variable::stack_variable_size,
				stack.raw.begin() + stack.offset,
				stack.raw.begin() + frames.top().stack_offset
			);

			stack.offset = frames.top().stack_offset + Variable::stack_variable_size;

			if constexpr (debug_mode)
			{
				fmt::print(
					fmt::color::blue_violet,
					"Returning from {}\n\n",
					script.name
				);
			}

			return;
		} break;

		// case Instr::opexit: // TODO
		// case Instr::oppopz: // TODO

		case Instr::opb: branch(); break;
		case Instr::opbt: if (stack.pop<bool>()) { branch(); } break;
		case Instr::opbf: if (!stack.pop<bool>()) { branch(); } break;

		// case Instr::oppushenv: // TODO
		// case Instr::oppopenv: // TODO

		//case Instr::oppushcst: break;

		//case Instr::oppushloc:
		//case Instr::oppushglb:

		case Instr::oppushspc:
			push_special(SpecialVar(*(++block) & 0x00FFFFFF));
			break;

		case Instr::oppushi16:
			stack.push<s32>(s16(*block & 0xFFFF));
			break;

		case Instr::opcall: {
			Frame& frame = frames.push();
			auto return_type = DataType((*block >> 16) & 0xFF);
			auto argument_count = *block & 0xFFFF;
			frame.stack_offset = stack.offset - argument_count * Variable::stack_variable_size;

			// TODO: reduce indirection here
			const auto& func = form.func.definitions[*(++block)];

			if (func.is_builtin)
			{
				throw std::runtime_error{"Unhandled call to builtin"};
			}
			else
			{
				execute(*func.associated_script);
			}

			frames.pop();
		} break;

		// case Instr::opbreak: // TODO

		default:
			fmt::print(fmt::color::red, "Unhandled op ${:02x}\n", opcode);
			throw std::runtime_error{"Reached unhandled operation in VM"};
		}

		++block;
	}
}

void VM::push_special(SpecialVar var)
{
	// argumentn
	if (unsigned(var) <= 16)
	{
		stack.push_raw(
			&stack.raw[frames.top().stack_offset + Variable::stack_variable_size * unsigned(var)],
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
