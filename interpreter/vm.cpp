#include "vm.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>
#include <type_traits>
#include "blockreader.hpp"
#include "traits.hpp"
#include "../util/nametype.hpp"

#define BINOP_ARITH(name, op) case Instr::name : \
	binop_arithmetic([&](auto a, auto b) { \
		return a op b; \
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

void VM::print_stack_frame()
{
	fmt::print(
		fmt::color::gray,
		"Stack frame data ({:5} bytes): {:02x}\n",
		stack.offset() - frames.top().stack_offset,
		fmt::join(
			std::vector<u8>(
				&stack.raw[frames.top().stack_offset],
				&stack.raw[stack.offset()]
			),
			""
		)
	);
}

void VM::execute(const Script& script)
{
	if constexpr (debug_mode)
	{
		fmt::print(
			fmt::color::red,
			"\nExecuting function '{}' (call depth {})",
			script.name,
			frames.offset + 1
		);
	}

	BlockReader reader{script};

	for (;;)
	{
		const auto block = reader.current_block();
		const auto opcode = Instr(block >> 24);
		const auto t1 = DataType((block >> 16) & 0xF);
		const auto t2 = DataType((block >> 20) & 0xF);

		if constexpr (debug_mode)
		{
			fmt::print(
				fmt::color::orange,
				"\nExecution trace: ${:08x}: ${:02x}. ",
				reader.offset(),
				u8(opcode)
			);

			print_stack_frame();
		}

		auto pop_parameter = [&](auto handler, DataType type) {
			if (type == DataType::var)
			{
				auto inst_type = stack.pop<InstType>();
				auto data_type = pop_variable_var_type(inst_type);
				return dispatcher([&](auto v) {
					VariableReference<decltype(v)> var{inst_type, data_type};
					return handler(var);
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
							"    f(pop<{}>(), pop<{}>())\n",
							type_name<decltype(a)>(),
							type_name<decltype(b)>()
						);
					}

					handler(a, b);
				}, t2);
			}, t1);
		};

		auto binop_arithmetic = [&](auto handler) {
			stack_dispatch_2([&](auto b, auto a) {
				if constexpr (are_arithmetic_convertible<decltype(a), decltype(b)>())
				{
					if constexpr (is_var<decltype(a)>()
							   || is_var<decltype(b)>())
					{
						if constexpr (debug_mode)
						{
							fmt::print(
								fmt::color::yellow_green,
								"    -> Variable<{}>\n",
								type_name<decltype(handler(value(a), value(b)))>()
							);
						}

						push_stack_variable(handler(value(a), value(b)));
					}
					else
					{
						if constexpr (debug_mode)
						{
							fmt::print(
								fmt::color::yellow_green,
								"    -> {}\n",
								type_name<decltype(handler(a, b))>()
							);
						}

						stack.push(handler(a, b));
					}
				}
			});
		};

		auto branch = [&] {
			s32 offset = block & 0xFFFFFF;

			if constexpr (debug_mode)
			{
				fmt::print(
					fmt::color::yellow_green,
					"    Branching with offset {} blocks\n",
					offset
				);
			}

			// TODO: make this nicer somehow? skipping block++ on the end
			reader.relative_jump(offset - 1);
		};

		switch (opcode)
		{
		case Instr::opconv:
			pop_parameter([&](auto src) {
				dispatcher([&](auto dst) {
					if constexpr (std::is_same_v<decltype(dst), VariablePlaceholder>)
					{
						push_stack_variable(src);
					}
					else if constexpr (std::is_arithmetic_v<decltype(dst)>
									&& is_arithmetic_convertible<decltype(src)>())
					{
						stack.push<decltype(dst)>(value(src));
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
			auto func = CompFunc((block >> 8) & 0xFF);
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
				}(func, value(a), value(b)));
			});
		} break;

		// case Instr::oppop: // TODO
		// case Instr::oppushi16: // TODO
		// case Instr::opdup: // TODO

		case Instr::opret: {
			std::move(
				stack.raw.begin() + stack.offset() - Variable::stack_variable_size,
				stack.raw.begin() + stack.offset(),
				stack.raw.begin() + frames.top().stack_offset
			);

			stack.skip(stack.offset() - frames.top().stack_offset - Variable::stack_variable_size);

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
			push_special(SpecialVar(reader.next_block() & 0x00FFFFFF));
			break;

		case Instr::oppushi16:
			stack.push<s32>(s16(block & 0xFFFF));
			break;

		case Instr::opcall: {
			Frame& frame = frames.push();
			auto argument_count = block & 0xFFFF;
			frame.stack_offset = stack.offset() - argument_count * Variable::stack_variable_size;

			// TODO: reduce indirection here
			const auto& func = form.func.definitions[reader.next_block()];

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
			fmt::print(fmt::color::red, "Unhandled op ${:02x}\n", u8(opcode));
			throw std::runtime_error{"Reached unhandled operation in VM"};
		}

		reader.next_block();
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
