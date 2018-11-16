#include "vm.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>
#include <type_traits>
#include "blockreader.hpp"
#include "traits.hpp"
#include "../util/cast.hpp"
#include "../util/compilersupport.hpp"
#include "../util/nametype.hpp"
#include "variablereference.hpp"

#define BINOP_ARITH(name, op) case Instr::name : \
	op_arithmetic2([&](auto a, auto b) { \
		if constexpr (are_arithmetic<decltype(a), decltype(b)>()) \
		{ \
			return a op b; \
		} \
		maybe_unreachable(); \
	}); \
	break;

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
	if constexpr (check(debug::vm_verbose_calls))
	{
		fmt::print(
			fmt::color::red,
			"\nExecuting function '{}' (call depth {}, initial stack frame size {})",
			script.name,
			frames.offset + 1,
			stack.offset() - frames.top().stack_offset
		);
	}

	BlockReader reader{script};

	for (;;)
	{
		const auto block = reader.current_block();
		const auto opcode = Instr(block >> 24);
		const auto t1 = DataType((block >> 16) & 0xF);
		const auto t2 = DataType((block >> 20) & 0xF);

		if constexpr (check(debug::vm_verbose_instructions))
		{
			fmt::print(
				fmt::color::orange,
				"\nExecution trace: ${:08x}: ${:02x}. ",
				reader.offset(),
				enum_value(opcode)
			);

			print_stack_frame();
		}

		auto pop_dispatch = [&](auto handler, DataType type) FORCE_INLINE {
			if (type == DataType::var)
			{
				auto inst_type = stack.pop<InstType>();
				auto data_type = pop_variable_var_type(inst_type);
				return dispatcher([&](auto v) {
					return handler(VariableReference<decltype(v)>{inst_type}.read(*this));
				}, std::array{data_type});
			}

			return dispatcher([&](auto v) {
				handler(stack.pop<decltype(v)>());
			}, std::array{type});
		};

		auto op_pop2 = [&](auto handler) FORCE_INLINE {
			pop_dispatch([&](auto a) {
				pop_dispatch([&](auto b) {
					if constexpr (check(debug::vm_verbose_instructions))
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

		auto op_arithmetic2 = [&](auto handler) FORCE_INLINE {
			op_pop2([&](auto b, auto a) {
				using ReturnType = decltype(handler(value(a), value(b)));

				if constexpr (!std::is_void_v<ReturnType>)
				{
					if constexpr (is_var<decltype(a)>() || is_var<decltype(b)>())
					{
						if constexpr (check(debug::vm_verbose_instructions))
						{
							fmt::print(
								fmt::color::yellow_green,
								"    -> Variable<{}>\n",
								type_name<ReturnType>()
							);
						}

						push_stack_variable(handler(value(a), value(b)));
					}
					else
					{
						if constexpr (check(debug::vm_verbose_instructions))
						{
							fmt::print(
								fmt::color::yellow_green,
								"    -> {}\n",
								type_name<ReturnType>()
							);
						}

						stack.push(handler(a, b));
					}
				}
			});
		};

		auto op_arithmetic_integral2 = [&](auto handler) FORCE_INLINE {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (std::is_integral_v<decltype(a)>
						   && std::is_integral_v<decltype(b)>)
				{
					return handler(a, b);
				}
			});
		};

		auto branch = [&]() FORCE_INLINE {
			s32 offset = block & 0xFFFFFF;

			if constexpr (check(debug::vm_verbose_instructions))
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
			pop_dispatch([&](auto src) FORCE_INLINE {
				dispatcher([&](auto dst) FORCE_INLINE {
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
						maybe_unreachable("Unimplemented conversion types");
					}
				}, std::array{t2});
			}, t1);
			break;

		case Instr::opmul: {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (std::is_integral_v<decltype(a)>
						   && std::is_same_v<decltype(b), StringReference>)
				{
					// TODO
				}

				if constexpr (are_arithmetic<decltype(a), decltype(b)>())
				{
					return a * b;
				}

				maybe_unreachable("Multiply op should be impossible");
			});

		} break;

		BINOP_ARITH(opdiv, /)
		// case Instr::oprem: // TODO
		// case Instr::opmod: // TODO

		case Instr::opadd: {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (std::is_same_v<decltype(a), StringReference>
						   && std::is_same_v<decltype(b), StringReference>)
				{
					// TODO
				}

				if constexpr (are_arithmetic<decltype(a), decltype(b)>())
				{
					return a + b;
				}
			});
		} break;

		BINOP_ARITH(opsub, -)

		case Instr::opand:
			op_arithmetic_integral2([&](auto a, auto b) { return a & b; });
			break;

		case Instr::opor:
			op_arithmetic_integral2([&](auto a, auto b) { return a | b; });
			break;

		case Instr::opxor:
			op_arithmetic_integral2([&](auto a, auto b) { return a ^ b; });
			break;

		// case Instr::opneg: // TODO
		// case Instr::opnot: // TODO

		case Instr::opshl: {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (std::is_integral_v<decltype(a)>
						   && std::is_integral_v<decltype(b)>)
				{
					return a << b;
				}
			});
		} break;

		case Instr::opshr: {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (std::is_integral_v<decltype(a)>
						   && std::is_integral_v<decltype(b)>)
				{
					return a >> b;
				}
			});
		} break;

		case Instr::opcmp: {
			auto func = CompFunc((block >> 8) & 0xFF);
			op_pop2([&](auto a, auto b) {
				stack.push([]([[maybe_unused]] auto func, auto a, auto b) FORCE_INLINE -> bool {
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
						default: maybe_unreachable();
						}
					}

					maybe_unreachable("Comparison should be impossible");
				}(func, value(a), value(b)));
			});
		} break;

		case Instr::oppop: {
			pop_dispatch([&](auto v) {
				auto inst_type = InstType((block >> 16) & 0xFF);
				VariableReference<decltype(value(v))> dst{inst_type};
				//AAAAAAA read variable ref and thing
				//write_variable(dst);
			}, t1);
		} break;

		// case Instr::oppushi16: // TODO
		// case Instr::opdup: // TODO

		case Instr::opret: {
			std::move(
				stack.raw.begin() + stack.offset() - Variable::stack_variable_size,
				stack.raw.begin() + stack.offset(),
				stack.raw.begin() + frames.top().stack_offset
			);

			stack.skip(stack.offset() - frames.top().stack_offset - Variable::stack_variable_size);

			if constexpr (check(debug::vm_verbose_calls))
			{
				fmt::print(
					fmt::color::blue_violet,
					"\nReturning from {}\n\n",
					script.name
				);
			}

			return;
		} break;

		// case Instr::opexit: // TODO

		case Instr::oppopz: {
			pop_dispatch([]([[maybe_unused]] auto v){}, t1);
		} break;

		case Instr::opb: branch(); break;
		case Instr::opbt: if (stack.pop<bool>()) { branch(); } break;
		case Instr::opbf: if (!stack.pop<bool>()) { branch(); } break;

		// case Instr::oppushenv: // TODO
		// case Instr::oppopenv: // TODO

		//case Instr::oppushcst: break;

		//case Instr::oppushloc:
		//case Instr::oppushglb:

		case Instr::oppushspc:
			read_special(SpecialVar(reader.next_block() & 0x00FFFFFF));
			break;

		case Instr::oppushi16:
			stack.push<s32>(s16(block & 0xFFFF));
			break;

		case Instr::opcall: {
			Frame& frame = frames.push();
			auto argument_count = block & 0xFFFF;
			frame.stack_offset = stack.offset() - argument_count * Variable::stack_variable_size;
			frame.max_local_count = script.local_count;

			// TODO: reduce indirection here
			const auto& func = form.func.definitions[reader.next_block()];

			if (func.is_builtin)
			{
				maybe_unreachable("Builtins are not implemented");
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
			maybe_unreachable("Reached unhandled operation in VM");
		}

		reader.next_block();
	}
}

void VM::read_special(SpecialVar var)
{
	// argumentn
	if (unsigned(var) <= 16)
	{
		Frame& frame = frames.top();
		stack.push_raw(
			&stack.raw[frame.stack_offset + Variable::stack_variable_size * (frame.max_local_count + unsigned(var))],
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
		maybe_unreachable("Unhandled variable type");
	}
}
