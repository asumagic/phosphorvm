#include "pvm/vm/vm.hpp"

#include "pvm/bc/disasm.hpp"
#include "pvm/util/cast.hpp"
#include "pvm/util/compilersupport.hpp"
#include "pvm/util/nametype.hpp"
#include "pvm/vm/blockreader.hpp"
#include "pvm/vm/traits.hpp"
#include "pvm/vm/variableoperand.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <stdexcept>
#include <type_traits>
#include <utility>

VarId VM::local_id_from_reference(u32 reference) const
{
	return form.vari.definitions[reference & 0x00FFFFFFu].unknown - 1;
}

void VM::print_stack_frame()
{
	std::vector<u8> frame(
	    &stack.raw[frames.top().stack_offset], &stack.raw[stack.offset]);

	fmt::print(
	    fmt::color::gray,
	    "Stack frame data ({:5} bytes): {:02x}\n",
	    stack.offset - frames.top().stack_offset,
	    fmt::join(frame, ""));
}

void VM::call(const FunctionDefinition& func, std::size_t argument_count)
{
	Frame& frame         = frames.push();
	frame.argument_count = argument_count;
	frame.stack_offset
	    = stack.offset - frame.argument_count * Variable::stack_variable_size;

	if (func.is_builtin)
	{
		func.associated_builtin(*this);
	}
	else
	{
		Script& called_script = *func.associated_script;
		stack.skip(-called_script.local_count * Variable::stack_variable_size);
		run(called_script);
	}

	frames.pop();
}

void VM::run(const Script& script)
{
	if constexpr (check(debug::vm_verbose_calls))
	{
		fmt::print(
		    fmt::color::red,
		    "Executing function '{}' ({}th nested call, {} bytes allocated on "
		    "stack)\n",
		    script.name,
		    frames.offset + 1,
		    stack.offset - frames.top().stack_offset);
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
			    disasm.disassemble_block(&script.data[reader.offset()], &script)
			        .as_plain_string(),
			    reader.offset(),
			    enum_value(state.opcode));
		}

		//! Jumps to another instruction with an offset defined by the current
		//! block.
		auto branch = [&]() FORCE_INLINE {
			s16 offset = state.block & 0xFFFFu;

			if constexpr (check(debug::vm_verbose_instructions))
			{
				fmt::print(
				    fmt::color::yellow_green,
				    "    Branching with offset {} blocks\n",
				    offset);
			}

			// TODO: make this nicer somehow? skipping block++ on the end
			reader.relative_jump(offset - 1);
		};

		switch (state.opcode)
		{
		case Instr::opconv:
		{
			pop_dispatch(
			    [&](auto src) FORCE_INLINE {
				    dispatcher(
				        [&](auto dst) FORCE_INLINE {
					        if constexpr (std::is_same_v<
					                          decltype(dst),
					                          VariablePlaceholder>)
					        {
						        push_stack_variable(src);
					        }
					        else if constexpr (are<std::is_arithmetic,
					                               decltype(dst),
					                               decltype(value(src))>())
					        {
						        stack.push<decltype(dst)>(value(src));
					        }
					        else
					        {
						        maybe_unreachable(
						            "Unimplemented conversion types");
					        }
				        },
				        std::array{state.t2});
			    },
			    state.t1);

			break;
		}

		case Instr::opmul:
		{
			op_arithmetic2(state, [&](auto a, auto b) {
				if constexpr (
				    std::is_integral_v<decltype(
				        a)> && std::is_same_v<decltype(b), StringReference>)
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

		case Instr::opdiv:
		{
			op_arithmetic_numeric2(state, [&](auto a, auto b) {
				// TODO: what to do on /0?
				return a / b;
			});

			break;
		}

			// case Instr::oprem: // TODO
			// case Instr::opmod: // TODO

		case Instr::opadd:
		{
			op_arithmetic2(state, [&](auto a, auto b) {
				if constexpr (
				    std::is_same_v<
				        decltype(a),
				        StringReference> && std::is_same_v<decltype(b), StringReference>)
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
			op_arithmetic_numeric2(
			    state, [&](auto a, auto b) { return a - b; });

			break;
		}

		case Instr::opand:
		{
			op_arithmetic_integral2(
			    state, [&](auto a, auto b) { return a & b; });

			break;
		}

		case Instr::opor:
		{
			op_arithmetic_integral2(
			    state, [&](auto a, auto b) { return a | b; });

			break;
		}

		case Instr::opxor:
		{
			op_arithmetic_integral2(
			    state, [&](auto a, auto b) { return a ^ b; });

			break;
		}

			// case Instr::opneg: // TODO
			// case Instr::opnot: // TODO

		case Instr::opshl:
		{
			op_arithmetic2(state, [&](auto a, auto b) {
				if constexpr (are<std::is_integral>(a, b))
				{
					return a << b;
				}
			});

			break;
		}

		case Instr::opshr:
		{
			op_arithmetic2(state, [&](auto a, auto b) {
				if constexpr (are<std::is_integral>(a, b))
				{
					return a >> b;
				}
			});

			break;
		}

		case Instr::opcmp:
		{
			auto func = CompFunc((state.block >> 8u) & 0xFFu);
			op_pop2(state, [&](auto a, auto b) {
				auto va = value(a);
				auto vb = value(b);

				if constexpr (are<std::is_arithmetic>(va, vb))
				{
					switch (func)
					{
					case CompFunc::lt: compare_flag = va < vb; break;
					case CompFunc::lte: compare_flag = va <= vb; break;
					case CompFunc::eq: compare_flag = va == vb; break;
					case CompFunc::neq: compare_flag = va != vb; break;
					case CompFunc::gte: compare_flag = va >= vb; break;
					case CompFunc::gt: compare_flag = va > vb; break;
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
			pop_dispatch(
			    [&](auto v) {
				    auto inst_type = InstType(s16(state.block & 0xFFFFu));
				    auto reference = reader.next_block();

				    write_variable(
				        inst_type,
				        reference & 0xFFFFFFu,
				        VarType(reference >> 24u),
				        value(v));
			    },
			    state.t2);

			break;
		}

			// case Instr::oppushi16: // TODO
			// case Instr::opdup: // TODO

		case Instr::opret:
		{
			std::move(
			    stack.raw.begin() + stack.offset
			        - Variable::stack_variable_size,
			    stack.raw.begin() + stack.offset,
			    stack.raw.begin() + frames.top().stack_offset);

			stack.skip(
			    stack.offset - frames.top().stack_offset
			    - Variable::stack_variable_size);

			if constexpr (check(debug::vm_verbose_calls))
			{
				fmt::print(
				    fmt::color::blue_violet,
				    "\nReturning from {}\n\n",
				    script.name);
			}

			return;
		}

			// case Instr::opexit: // TODO

		case Instr::oppopz:
		{
			pop_dispatch([]([[maybe_unused]] auto v) {}, state.t1);
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
			dispatcher(
			    [&](auto v) {
				    if constexpr (std::is_arithmetic_v<decltype(v)>)
				    {
					    if (state.t1 == DataType::i16)
					    {
						    stack.push(s32(state.block & 0xFFFFu));
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
				    else if constexpr (std::is_same_v<
				                           decltype(v),
				                           VariablePlaceholder>)
				    {
					    auto inst_type = InstType(s16(state.block & 0xFFFFu));
					    auto reference = reader.next_block();

					    switch (inst_type)
					    {
					    case InstType::local:
						    stack.push_raw(
						        &stack.raw[frames.top().local_offset(
						            local_id_from_reference(reference))],
						        Variable::stack_variable_size);
						    break;

					    default:
						    maybe_unreachable(
						        "InstType not implemented for pushcst");
					    }
				    }
				    else
				    {
					    maybe_unreachable("Type not implemented for pushcst");
				    }
			    },
			    std::array{state.t1});
			break;
		}

		case Instr::oppushloc:
		{
			auto reference = reader.next_block();
			stack.push_raw(
			    &stack.raw[frames.top().local_offset(
			        local_id_from_reference(reference))],
			    Variable::stack_variable_size);
			break;
		}

			// case Instr::oppushglb:

		case Instr::oppushspc:
		{
			read_special(SpecialVar(reader.next_block() & 0x00FFFFFFu));
			break;
		}

		case Instr::oppushi16:
		{
			stack.push<s32>(s16(state.block & 0xFFFFu));
			break;
		}

		case Instr::opcall:
		{
			std::size_t argument_count = state.block & 0xFFFFu;
			auto&       func = form.func.definitions[reader.next_block()];
			call(func, argument_count);
			break;
		}

			// case Instr::opbreak: // TODO

		default:
		{
			fmt::print(
			    fmt::color::red, "Unhandled op ${:02x}\n", u8(state.opcode));
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
		    Variable::stack_variable_size);

		return;
	}
}

void VM::type_error()
{
	throw std::runtime_error{"Type check error!\n"};
}
