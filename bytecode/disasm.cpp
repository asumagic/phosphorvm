#include "../unpack/reader.hpp"
#include "disasm.hpp"
#include "types.hpp"
#include <fmt/core.h>
#include <fmt/color.h>
#include <string_view>

const std::string& Disassembler::get_string(s32 id)
{
	return form.strg.elements[id].value;
}

std::string Disassembler::type_suffix(u32 type)
{
	switch (type)
	{
	case 0x0: return "f64";
	case 0x1: return "f32";
	case 0x2: return "i32";
	case 0x3: return "i64";
	case 0x4: return "b8";
	case 0x5: return "var";
	case 0x6: return "str";
	case 0xF: return "i8";
	}

	return ".???";
}

std::string Disassembler::instance_name(InstId id)
{
	if (id > 0)
	{
		return fmt::to_string(id);
	}

	switch (id)
	{
		case  0: return "<unimpl>";
		case -1: return "self";
		case -2: return "other";
		case -3: return "all";
		case -4: return "noone";
		case -5: return "global";
	}

	return "<bad>";
}

std::string Disassembler::resolve_variable_name(s32 id)
{
	id &= 0x00FFFFFF;

	auto& vars = form.vari.definitions;

	if (id < 0 || size_t(id) >= vars.size())
	{
		return "<unexisting>";
	}

	return vars[id].name;
}

// TODO: genericize and merge with resolve_variable_name
std::string Disassembler::resolve_function_name(s32 func_id)
{
	if (func_id < 0 || size_t(func_id) >= form.func.definitions.size())
	{
		return "<unexisting>";
	}

	return form.func.definitions[func_id].name;
}

std::string Disassembler::comparator_name(u8 function)
{
	switch (function)
	{
	case 0x1: return "<";
	case 0x2: return "<=";
	case 0x3: return "==";
	case 0x4: return "!=";
	case 0x5: return ">=";
	case 0x6: return ">";
	}

	return "<bad>";
}

void Disassembler::operator()(const Script& script)
{
	auto& program = script.data;

	fmt::print(fmt::color::orange, "\nDisassembly of '{}': {} blocks ({} bytes)\n", script.name, program.size(), program.size() * 4);

	const Block* block_ptr = script.data.data();

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

		auto generic_2t = [&](auto name) {
			mnemonic = fmt::format("{}.{}.{}", name, type_suffix(t1), type_suffix(t2));
		};

		auto read_block_operand = [&](auto& into) {
			std::memcpy(&into, block_ptr, sizeof(decltype(into)));
			block_ptr += sizeof(decltype(into)) / sizeof(Block);
			return into;
		};

		auto push_param = [&](auto type) -> std::string {
			switch (type)
			{
			case 0x0: { f32 v; return fmt::to_string(read_block_operand(v)); }
			case 0x1: { f64 v; return fmt::to_string(read_block_operand(v)); }
			case 0x2: { s32 v; return fmt::to_string(read_block_operand(v)); }
			case 0x3: { s64 v; return fmt::to_string(read_block_operand(v)); }
			case 0x4: { s32 v; return fmt::to_string(bool(read_block_operand(v))); }
			case 0x5: { u32 v; return fmt::to_string(resolve_variable_name(read_block_operand(v) & 0xFFFFFF)); }
			case 0x6: { s32 v; return fmt::format("\"{}\"", get_string(read_block_operand(v))); }
			case 0xF: return fmt::to_string(main_block & 0xffff);
			}

			return "<unknown>";
		};

		switch (op)
		{
		case 0x07: generic_2t("conv"); break;

		case 0x08: generic_2t("mul"); break;
		case 0x09: generic_2t("div"); break;
		case 0x0A: generic_2t("rem"); break;
		case 0x0B: generic_2t("mod"); break;
		case 0x0C: generic_2t("add"); break;
		case 0x0D: generic_2t("sub"); break;
		case 0x0E: generic_2t("and"); break;
		case 0x0F: generic_2t("or"); break;
		case 0x10: generic_2t("xor"); break;

		case 0x11: break;

		case 0x12: generic_2t("not"); break;
		case 0x13: generic_2t("shl"); break;
		case 0x14: generic_2t("shr"); break;

		case 0x15: {
			mnemonic = fmt::format("cmp.{}.{}", type_suffix(t1), type_suffix(t2));
			params = comparator_name(u8((main_block >> 8) & 0xFF));
		} break;

		case 0x45: {
			mnemonic = fmt::format("pop.{}.{}", type_suffix(t1), type_suffix(t2));
			params = resolve_variable_name(*(block_ptr++));
		} break;

		case 0x9C: {
			mnemonic = fmt::format("ret.{}", type_suffix(t1));
		} break;

		case 0x9D: {
			mnemonic = "exit";
		} break;

		case 0x9E: break;

		case 0xB6: break;
		case 0xB7: break;
		case 0xB8: break;

		case 0xBA: break;
		case 0xBB: break;

		case 0xC0: {
			mnemonic = fmt::format("pushcst.{}", type_suffix(t1));
			params = push_param(t1);
		} break;

		case 0xC1: {
			mnemonic = fmt::format("pushloc.{}", type_suffix(t1));
			params = push_param(t1);
		} break;

		case 0xC2: break;

		case 0xC3: {
			mnemonic = fmt::format("pushvar.{}", type_suffix(t2));
			params = push_param(t1);
		} break;

		case 0x84: {
			mnemonic = "push.i16";
			params = fmt::to_string(main_block & 0xFFFF);
		} break;

		case 0xD9: {
			mnemonic = fmt::format("call.{}", type_suffix(t1));
			params = resolve_function_name(*(block_ptr++));
		} break;

		case 0xFF: {
			mnemonic = "break";
			params = fmt::to_string(static_cast<s16>(main_block & 0xFFFF));
		} break;

		default: {
			mnemonic = "<bad>";
			comment = "!!! This may indicate corruption";
		}

		};

		bool mnemonic_warning = !mnemonic.empty() && mnemonic[0] == '<';
		bool params_warning = !params.empty() && params[0] == '<';

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
