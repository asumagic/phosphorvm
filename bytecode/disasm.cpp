#include "disasm.hpp"
#include "types.hpp"
#include "enums.hpp"
#include <fmt/core.h>
#include <fmt/color.h>
#include <string_view>

std::string Disassembler::get_string(s32 id)
{
	if (id < 0 || size_t(id) >= _form.strg.elements.size())
	{
		return "<badstr>";
	}
	return _form.strg.elements[id].value;
}

std::string Disassembler::type_suffix(u32 type)
{
	switch (DataType(type))
	{
	case DataType::f64: return "f64";
	case DataType::f32: return "f32";
	case DataType::i32: return "i32";
	case DataType::i64: return "i64";
	case DataType::b32: return "b32";
	case DataType::var: return "var";
	case DataType::str: return "str";
	case DataType::i16: return "i16";
	}

	return ".???";
}

std::string Disassembler::instance_name(InstId id)
{
	if (id > 0)
	{
		return fmt::to_string(id);
	}

	switch (InstType(id))
	{
		case InstType::stack_top_or_global: return "(stacktop)";
		case InstType::self:                return "self";
		case InstType::other:               return "other";
		case InstType::all:                 return "all";
		case InstType::noone:               return "noone";
		case InstType::global:              return "global";
		case InstType::local:               return "local";
	}

	return "<bad>";
}

std::string Disassembler::resolve_variable_name(s32 id)
{
	id &= 0x00FFFFFF;

	auto& vars = _form.vari.definitions;

	if (id < 0 || size_t(id) >= vars.size())
	{
		return "<unexisting>";
	}

	return vars[id].name;
}

// TODO: genericize and merge with resolve_variable_name
std::string Disassembler::resolve_function_name(s32 func_id)
{
	if (func_id < 0 || size_t(func_id) >= _form.func.definitions.size())
	{
		return "<unexisting>";
	}

	return _form.func.definitions[func_id].name;
}

std::string Disassembler::comparator_name(u8 function)
{
	switch (CompFunc(function))
	{
	case CompFunc::lt:  return "<";
	case CompFunc::lte: return "<=";
	case CompFunc::eq:  return "==";
	case CompFunc::neq: return "!=";
	case CompFunc::gte: return ">=";
	case CompFunc::gt:  return ">";
	}

	return "<bad>";
}

void Disassembler::operator()(const Script& script)
{
	auto& program = script.data;

	fmt::print(fmt::color::orange, "\nDisassembly of '{}': {} blocks ({} bytes)\n", script.name, program.size(), program.size() * 4);

	block_ptr = script.data.data();

	while (block_ptr != &(*script.data.end()))
	{
		const Block* main_block_ptr = block_ptr++;
		auto main_block = *main_block_ptr;

		auto op = (main_block >> 24) & 0xFF;

		// Type pairs
		auto t1 = (main_block >> 16) & 0xF;
		auto t2 = (main_block >> 20) & 0xF;

		fmt::print(
			fmt::color::light_gray,
			"0x{:08x}: ",
			std::distance(script.data.data(), main_block_ptr),
			main_block
		);

		std::string mnemonic = "<unimpl>", params, comment;

		auto read_block_operand = [&](auto& into) {
			std::memcpy(&into, block_ptr, sizeof(decltype(into)));
			block_ptr += sizeof(decltype(into)) / sizeof(Block);
			return into;
		};

		auto push_param = [&](auto type) -> std::string {
			switch (DataType(type))
			{
			case DataType::f64: { f64 v; return fmt::to_string(read_block_operand(v)); }
			case DataType::f32: { f32 v; return fmt::to_string(read_block_operand(v)); }
			case DataType::i32: { s32 v; return fmt::to_string(read_block_operand(v)); }
			case DataType::i64: { s64 v; return fmt::to_string(read_block_operand(v)); }
			case DataType::b32: { s32 v; return fmt::to_string(bool(read_block_operand(v))); }
			case DataType::var: { u32 v; return fmt::to_string(resolve_variable_name(read_block_operand(v) & 0xFFFFFF)); }
			case DataType::str: { s32 v; return fmt::format("\"{}\"", get_string(read_block_operand(v))); }
			case DataType::i16: return fmt::to_string(main_block & 0xffff);
			}

			return "<unknown>";
		};

		auto generic_1t = [&](auto name) {
			mnemonic = fmt::format("{}.{}", name, type_suffix(t1));
		};

		auto generic_2t = [&](auto name) {
			mnemonic = fmt::format("{}.{}.{}", name, type_suffix(t1), type_suffix(t2));
		};

		auto generic_push = [&](auto name, auto type) {
			mnemonic = fmt::format("{}.{}", name, type_suffix(type));
			params = push_param(t1);
		};

		// TODO: add labels for easier disassembly read
		auto generic_goto = [&](auto name) {
			mnemonic = name;
		};

		switch (Instr(op))
		{
		case Instr::opconv:     generic_2t("conv"); break;
		case Instr::opmul:      generic_2t("mul"); break;
		case Instr::opdiv:      generic_2t("div"); break;
		case Instr::oprem:      generic_2t("rem"); break;
		case Instr::opmod:      generic_2t("mod"); break;
		case Instr::opadd:      generic_2t("add"); break;
		case Instr::opsub:      generic_2t("sub"); break;
		case Instr::opand:      generic_2t("and"); break;
		case Instr::opor:       generic_2t("or"); break;
		case Instr::opxor:      generic_2t("xor"); break;
		case Instr::opshl:      generic_2t("shl"); break;
		case Instr::opshr:      generic_2t("shr"); break;

		case Instr::opneg:      generic_1t("neg"); break;
		case Instr::opnot:      generic_1t("not"); break;
		case Instr::opret:      generic_1t("ret"); break;
		case Instr::oppopz:     generic_1t("popz"); break;
		case Instr::opdup:      generic_1t("dup"); break;

		case Instr::opexit:     mnemonic = "exit"; break;

		case Instr::opb:        generic_goto("b"); break;
		case Instr::opbt:       generic_goto("bt"); break;
		case Instr::opbf:       generic_goto("bf"); break;
		case Instr::oppushenv:  generic_goto("pushenv"); break;
		case Instr::oppopenv:   generic_goto("popenv"); break;

		case Instr::oppushcst:  generic_push("pushcst", t1); break;
		case Instr::oppushloc:  generic_push("pushloc", t1); break;
		case Instr::oppushglb:  generic_push("pushglb", t1); break;
		case Instr::oppushvar:  generic_push("pushvar", t2); break;

		case Instr::oppushi16: {
			mnemonic = "push.i16";
			params = fmt::to_string(main_block & 0xFFFF);
		} break;

		case Instr::opcmp: {
			mnemonic = fmt::format("cmp.{}.{}", type_suffix(t1), type_suffix(t2));
			params = comparator_name(u8((main_block >> 8) & 0xFF));
		} break;

		// TODO: pop.i16.var seems to cause issues
		case Instr::oppop: {
			mnemonic = fmt::format("pop.{}.{}", type_suffix(t1), type_suffix(t2));
			params = resolve_variable_name(*(block_ptr++));
		} break;

		case Instr::opcall: {
			mnemonic = fmt::format("call.{}", type_suffix(t1));
			params = resolve_function_name(*(block_ptr++));
		} break;

		case Instr::opbreak: {
			mnemonic = "break";
			params = fmt::to_string(static_cast<s16>(main_block & 0xFFFF));
		} break;

		default: {
			mnemonic = "<bad>";
			comment = "!!! This may indicate corruption";
		}

		};

		// HACK -y stuff. probably refactor that part?
		bool mnemonic_warning = !mnemonic.empty() && mnemonic[0] == '<';
		bool params_warning = params.size() >= 3 && params[0] == '<';

		comment.insert(0, fmt::format(
			"${:08x} ",
			fmt::join(std::vector<Block>(main_block_ptr, block_ptr), "'")
		));

		if (mnemonic_warning)
		{
			fmt::print(fmt::color::brown, "{:15}", mnemonic);
		}
		else
		{
			fmt::print("{:15}", mnemonic);
		}

		if (params_warning)
		{
			fmt::print(fmt::color::brown, "{:20}", params);
		}
		else
		{
			fmt::print("{:20}", params);
		}

		fmt::print(fmt::color::gray, "; {}\n", comment);
	}
}
