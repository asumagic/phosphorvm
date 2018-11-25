#include "pvm/vm/vm.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include <utility>
#include <type_traits>
#include "pvm/bc/disasm.hpp"
#include "pvm/vm/blockreader.hpp"
#include "pvm/vm/traits.hpp"
#include "pvm/vm/variableoperand.hpp"
#include "pvm/vm/vmstate.hpp"
#include "pvm/util/cast.hpp"
#include "pvm/util/compilersupport.hpp"
#include "pvm/util/nametype.hpp"

VarId VM::local_id_from_reference(u32 reference) const
{
	return form.vari.definitions[reference & 0x00FFFFFF].unknown - 1;
}

void VM::print_stack_frame()
{
	std::vector<u8> frame(
		&stack.raw[frames.top().stack_offset],
		&stack.raw[stack.offset]
	);

	fmt::print(
		fmt::color::gray,
		"Stack frame data ({:5} bytes): {:02x}\n",
		stack.offset - frames.top().stack_offset,
		fmt::join(frame, "")
	);
}

void VM::call(const FunctionDefinition& func, std::size_t argument_count)
{
	Frame& frame = frames.push();
	frame.argument_count = argument_count;
	frame.stack_offset = stack.offset - frame.argument_count * Variable::stack_variable_size;

	if (func.is_builtin)
	{
		func.associated_builtin(*this);
	}
	else
	{
		Script& called_script = *func.associated_script;
		stack.skip(-called_script.local_count * Variable::stack_variable_size);
		execute(called_script);
	}

	frames.pop();
}

void VM::execute(const Script& script)
{
	if constexpr (check(debug::vm_verbose_calls))
	{
		fmt::print(
			fmt::color::red,
			"Executing function '{}' ({}th nested call, {} bytes allocated on stack)\n",
			script.name,
			frames.offset + 1,
			stack.offset - frames.top().stack_offset
		);
	}

	BlockReader reader{script};

	bool compare_flag = false;

	for (;;)
	{
		VMState state{reader};

		if constexpr (check(debug::vm_verbose_instructions))
		{
			Disassembler disasm{form};

			print_stack_frame();

			fmt::print(
				fmt::color::orange,
				"{:80}\n",
				disasm.disassemble_block(&script.data[reader.offset()], &script).as_plain_string(),
				reader.offset(),
				enum_value(state.opcode)
			);
		}

		//! Calls a handler providing it a value of the given type popped from
		//! the stack.
		//! When encountering variables, will provide a VariableReference<T>
		//! with T being the variable type as read on the stack.
		auto pop_dispatch = [&](auto handler, DataType type) FORCE_INLINE {
			if (type == DataType::var)
			{
				return dispatcher([&](auto v) {
					return handler(read_variable_parameter<decltype(v)>());
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
				}, state.t2);
			}, state.t1);
		};

		//! Executes 'handler' as an arithmetic instruction.
		//! When either of the parameters is of variable type, the resulting
		//! type is always a stack variable.
		auto op_arithmetic2 = [&](auto handler) FORCE_INLINE {
			op_pop2([&](auto a, auto b) {
				using ReturnType = decltype(handler(value(a), value(b)));

				if constexpr (!std::is_void_v<ReturnType>)
				{
					if constexpr (is_var(a) || is_var(b))
					{
						if constexpr (check(debug::vm_verbose_instructions))
						{
							fmt::print(
								fmt::color::yellow_green,
								"    -> Variable<{}>\n",
								type_name<ReturnType>()
							);
						}

						auto va = value(a);
						auto vb = value(b);
						push_stack_variable(handler(va, vb));
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
					auto va = value(a);
					auto vb = value(b);
					return handler(va, vb);
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
			s16 offset = state.block & 0xFFFF;

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

		switch (state.opcode)
		{
		case Instr::opconv:
		{
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
				}, std::array{state.t2});
			}, state.t1);

			break;
		}

		case Instr::opmul:
		{
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

			break;
		}

		case Instr::opdiv: {
			op_arithmetic_numeric2([&](auto a, auto b) {
				// TODO: what to do on /0?
				return a / b;
			});

			break;
		}

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

			break;
		}

		case Instr::opsub:
		{
			op_arithmetic_numeric2([&](auto a, auto b) {
				return a - b;
			});

			break;
		}

		case Instr::opand:
		{
			op_arithmetic_integral2([&](auto a, auto b) {
				return a & b;
			});

			break;
		}

		case Instr::opor:
		{
			op_arithmetic_integral2([&](auto a, auto b) {
				return a | b;
			});

			break;
		}

		case Instr::opxor:
		{
			op_arithmetic_integral2([&](auto a, auto b) {
				return a ^ b;
			});

			break;
		}

		// case Instr::opneg: // TODO
		// case Instr::opnot: // TODO

		case Instr::opshl:
		{
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (are<std::is_integral>(a, b))
				{
					return a << b;
				}
			});

			break;
		}

		case Instr::opshr:
		{
			op_arithmetic2([&](auto a, auto b) {
				if constexpr (are<std::is_integral>(a, b))
				{
					return a >> b;
				}
			});

			break;
		}

		case Instr::opcmp:
		{
			auto func = CompFunc((state.block >> 8) & 0xFF);
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

			break;
		}

		case Instr::oppop:
		{
			pop_dispatch([&](auto v) {
				auto inst_type = InstType(s16(state.block & 0xFFFF));
				auto reference = reader.next_block();

				write_variable(
					inst_type,
					reference & 0xFFFFFF,
					VarType(reference >> 24),
					value(v)
				);
			}, state.t2);

			break;
		}

		// case Instr::oppushi16: // TODO
		// case Instr::opdup: // TODO

		case Instr::opret:
		{
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
		}

		// case Instr::opexit: // TODO

		case Instr::oppopz:
		{
			pop_dispatch([]([[maybe_unused]] auto v){}, state.t1);
			break;
		}

		case Instr::opb:
		{
			branch();
			break;
		}

		case Instr::opbt:
		{
			if (compare_flag)
			{
				branch();
			}

			break;
		}

		case Instr::opbf:
		{
			if (!compare_flag)
			{
				branch();
			}
			break;
		}

		// case Instr::oppushenv: // TODO
		// case Instr::oppopenv: // TODO

		case Instr::oppushcst:
		{
			dispatcher([&](auto v) {
				if constexpr (std::is_arithmetic_v<decltype(v)>)
				{
					if (state.t1 == DataType::i16)
					{
						stack.push(s32(state.block & 0xFFFF));
					}
					else
					{
						stack.push_raw(&reader.next_block(), sizeof(v));

						// Move to the last block of the instruction
						// TODO: reader.skip() or something
						for (unsigned i = 0; i < (sizeof(v) / 4) - 1; ++i)
						{
							reader.next_block();
						}
					}
				}
				else if constexpr (std::is_same_v<decltype(v), VariablePlaceholder>)
				{
					auto inst_type = InstType(s16(state.block & 0xFFFF));
					auto reference = reader.next_block();

					switch (inst_type)
					{
					case InstType::local:
						stack.push_raw(
							&stack.raw[frames.top().local_offset(local_id_from_reference(reference))],
							Variable::stack_variable_size
						);
						break;

					default:
						maybe_unreachable("InstType not implemented for pushcst");
					}
				}
				else
				{
					maybe_unreachable("Type not implemented for pushcst");
				}
			}, std::array{state.t1});
			break;
		}

		case Instr::oppushloc:
		{
			auto reference = reader.next_block();
			stack.push_raw(
				&stack.raw[frames.top().local_offset(local_id_from_reference(reference))],
				Variable::stack_variable_size
			);
			break;
		}

		//case Instr::oppushglb:

		case Instr::oppushspc:
		{
			read_special(SpecialVar(reader.next_block() & 0x00FFFFFF));
			break;
		}

		case Instr::oppushi16:
		{
			stack.push<s32>(s16(state.block & 0xFFFF));
			break;
		}

		case Instr::opcall:
		{
			std::size_t argument_count = state.block & 0xFFFF;
			auto& func = form.func.definitions[reader.next_block()];
			call(func, argument_count);
			break;
		}

		// case Instr::opbreak: // TODO

		default:
		{
			fmt::print(fmt::color::red, "Unhandled op ${:02x}\n", u8(state.opcode));
			maybe_unreachable("Reached unhandled operation in VM");
			break;
		}
		}

		reader.next_block();
		if (reader.out_of_bounds())
		{
			return;
		}
	}
}

void VM::read_special(SpecialVar var)
{
	// argumentn
	if (unsigned(var) <= 16)
	{
		stack.push_raw(
			&stack.raw[frames.top().argument_offset(unsigned(var))],
			Variable::stack_variable_size
		);

		return;
	}
}
