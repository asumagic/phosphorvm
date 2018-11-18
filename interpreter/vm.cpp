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
#include "variableoperand.hpp"

std::size_t VM::argument_offset(ArgId arg_id) const
{
	return frames.top().stack_offset + arg_id * Variable::stack_variable_size;
}

std::size_t VM::local_offset(VarId var_id) const
{
	auto first_local_offset = argument_offset(frames.top().argument_count);
	return first_local_offset + var_id * Variable::stack_variable_size;
}

void VM::print_stack_frame()
{
	fmt::print(
		fmt::color::gray,
		"Stack frame data ({:5} bytes): {:02x}\n",
		stack.offset - frames.top().stack_offset,
		fmt::join(
			std::vector<u8>(
				&stack.raw[frames.top().stack_offset],
				&stack.raw[stack.offset]
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
			"\nExecuting function '{}' ({}th nested call, {} bytes allocated on stack)",
			script.name,
			frames.offset + 1,
			stack.offset - frames.top().stack_offset
		);
	}

	BlockReader reader{script};

	bool compare_flag = false;

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

		//! Calls a handler providing it a value of the given type popped from
		//! the stack.
		//! When encountering variables, will provide a VariableReference<T>
		//! with T being the variable type as read on the stack.
		auto pop_dispatch = [&](auto handler, DataType type) FORCE_INLINE {
			if (type == DataType::var)
			{
				return dispatcher([&](auto v) {
					return handler(read_variable_reference<decltype(v)>());
				}, std::array{stack.pop<DataType>()});
			}

			return dispatcher([&](auto v) {
				handler(stack.pop<decltype(v)>());
			}, std::array{type});
		};

		//! Executes 'handler' as an instruction that pops two parameters.
		auto op_pop2 = [&](auto handler) FORCE_INLINE {
			// Parameters are correctly reversed here
			pop_dispatch([&](auto b) {
				pop_dispatch([&](auto a) {
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

		//! Executes 'handler' as an arithmetic instruction.
		//! When either of the parameters is of variable type, the resulting
		//! type is always a stack variable.
		auto op_arithmetic2 = [&](auto handler) FORCE_INLINE {
			op_pop2([&](auto a, auto b) {
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
				else
				{
					maybe_unreachable(
						"Provided function does not handle arithmetic between"
						"the two provided types"
					);
				}
			});
		};

		//! Executes 'handler' as an arithmetic instruction, but filters
		//! non-numeric parameters.
		auto op_arithmetic_numeric2 = [&](auto handler) FORCE_INLINE {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (are<std::is_arithmetic>(a, b))
				{
					return handler(value(a), value(b));
				}
			});
		};

		//! Executes 'handler' as an arithmetic instruction, but filters
		//! non-integral parameters.
		//! @see op_arithmetic2
		auto op_arithmetic_integral2 = [&](auto handler) FORCE_INLINE {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (are<std::is_integral>(a, b))
				{
					return handler(a, b);
				}
			});
		};

		//! Jumps to another instruction with an offset defined by the current
		//! block.
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
					else if constexpr (are<std::is_arithmetic, decltype(dst), decltype(value(src))>())
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

				if constexpr (are<std::is_arithmetic>(a, b))
				{
					return a * b;
				}

				maybe_unreachable("Multiply op should be impossible");
			});

		} break;

		case Instr::opdiv: {
			op_arithmetic_numeric2([&](auto a, auto b) {
				// TODO: what to do on /0?
				return a / b;
			});
		} break;

		// case Instr::oprem: // TODO
		// case Instr::opmod: // TODO

		case Instr::opadd: {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (std::is_same_v<decltype(a), StringReference>
						   && std::is_same_v<decltype(b), StringReference>)
				{
					// TODO
				}

				if constexpr (are<std::is_arithmetic>(a, b))
				{
					return a + b;
				}
			});
		} break;

		case Instr::opsub: {
			op_arithmetic_numeric2([&](auto a, auto b) {
				return a - b;
			});
		}

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
				if constexpr (are<std::is_integral>(a, b))
				{
					return a << b;
				}
			});
		} break;

		case Instr::opshr: {
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (are<std::is_integral>(a, b))
				{
					return a >> b;
				}
			});
		} break;

		case Instr::opcmp: {
			auto func = CompFunc((block >> 8) & 0xFF);
			op_pop2([&](auto a, auto b) {
				auto va = value(a);
				auto vb = value(b);

				if constexpr (are<std::is_arithmetic>(va, vb))
				{
					switch (func)
					{
					case CompFunc::lt:  compare_flag = va <  vb; break;
					case CompFunc::lte: compare_flag = va <= vb; break;
					case CompFunc::eq:  compare_flag = va == vb; break;
					case CompFunc::neq: compare_flag = va != vb; break;
					case CompFunc::gte: compare_flag = va >= vb; break;
					case CompFunc::gt:  compare_flag = va >  vb; break;
					default: maybe_unreachable();
					}
				}
				else
				{
					maybe_unreachable("Comparison should be impossible");
				}
			});
		} break;

		/*case Instr::oppop: {
			pop_dispatch([&](auto v) {
				auto inst_type = InstType((block >> 16) & 0xFF);
				auto reference = reader.next_block();
				ReadVariableReference<decltype(value(v))> dst{inst_type, s32(reference & 0xFFFFFF)};
				//AAAAAAA read variable ref and thing
				//write_variable(dst);
			}, t2);
		} break;*/

		// case Instr::oppushi16: // TODO
		// case Instr::opdup: // TODO

		case Instr::opret: {
			std::move(
				stack.raw.begin() + stack.offset - Variable::stack_variable_size,
				stack.raw.begin() + stack.offset,
				stack.raw.begin() + frames.top().stack_offset
			);

			stack.skip(stack.offset - frames.top().stack_offset - Variable::stack_variable_size);

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
		case Instr::opbt: if (compare_flag) { branch(); } break;
		case Instr::opbf: if (!compare_flag) { branch(); } break;

		// case Instr::oppushenv: // TODO
		// case Instr::oppopenv: // TODO

		case Instr::oppushcst:
		{
			dispatcher([&](auto v) {
				if constexpr (std::is_arithmetic_v<decltype(v)>)
				{
					if (t1 == DataType::i16)
					{
						stack.push(s32(block & 0xFFFF));
					}
					else
					{
						maybe_unreachable("Arithmetic type not implemented for pushcst");
					}
				}
				/*else if constexpr (std::is_same_v<decltype(v), VariablePlaceholder>)
				{
					auto info = InstType(s16(block & 0xFFFF));

					dispatcher([&](auto v) {
						VariableReference<decltype(v)> ref;
						push_stack_variable(ref.read(*this));
					}, std::array{info.data_type});
				}*/
				else
				{
					maybe_unreachable("Type not implemented for pushcst");
				}
			}, std::array{t1});
		} break;

		/*case Instr::oppushloc: {
			// TODO: be able to read data type then value in here
			//push_stack_variable();
		} break;*/

		//case Instr::oppushglb:

		case Instr::oppushspc:
			read_special(SpecialVar(reader.next_block() & 0x00FFFFFF));
			break;

		case Instr::oppushi16:
			stack.push<s32>(s16(block & 0xFFFF));
			break;

		case Instr::opcall: {
			Frame& frame = frames.push();
			frame.argument_count = block & 0xFFFF;
			frame.stack_offset = stack.offset - frame.argument_count * Variable::stack_variable_size;

			// TODO: reduce indirection here
			const auto& func = form.func.definitions[reader.next_block()];

			if (func.is_builtin)
			{
				maybe_unreachable("Builtins are not implemented");
			}
			else
			{
				Script& called_script = *func.associated_script;
				stack.skip(-called_script.local_count * Variable::stack_variable_size);
				execute(called_script);
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
		stack.push_raw(
			&stack.raw[argument_offset(unsigned(var))],
			Variable::stack_variable_size
		);

		return;
	}
}
