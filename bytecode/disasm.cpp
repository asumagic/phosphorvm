#include "../unpack/reader.hpp"
#include "disasm.hpp"
#include "types.hpp"
#include <fmt/core.h>
#include <fmt/color.h>

const std::string& get_string(s32 id, Form& form)
{
	return form.strg.elements[id].value;
}

std::string type_suffix(s8 type)
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

std::string instance_name(InstId id)
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

std::string resolve_variable_name(s32 variable_id, Form& form)
{
	auto& vars = form.vari.definitions;

	if (variable_id < 0 || size_t(variable_id) >= vars.size())
	{
		return "<unexisting>";
	}

	return vars[variable_id].name;
}

void print_disassembly(Form& form, const Script& script)
{
	auto& program = script.data;

	fmt::print(fmt::color::orange, "\nDisassembly of '{}': {} blocks ({} bytes)\n", script.name, program.size(), program.size() * 4);

	GenericReader<Block> reader{program.data()};

	while (reader.offset() < program.size())
	{
		GenericReader old_reader = reader;

		auto main_block = reader.read_pod<Block>();

		auto op = (main_block >> 24) & 0xFF;

		// Type pairs
		auto t1 = (main_block >> 16) & 0xF;
		auto t2 = (main_block >> 20) & 0xF;

		fmt::print(fmt::color::light_gray, "0x{:08x}: ", reader.bytes(), main_block);

		std::string mnemonic = "<unimpl>", params, comment;

		auto generic_2t = [&](auto name) {
			mnemonic = fmt::format("{}.{}.{}", name, type_suffix(t1), type_suffix(t2));
		};

		auto push_param = [&](auto type) -> std::string {
			switch (type)
			{
			case 0x0: return fmt::to_string(reader.read_pod<f64>());
			case 0x1: return fmt::to_string(reader.read_pod<f32>());
			case 0x2: return fmt::to_string(reader.read_pod<s32>());
			case 0x3: return fmt::to_string(reader.read_pod<s64>());
			case 0x4: return fmt::to_string(bool(reader.read_pod<Block>()));
			case 0x5: return fmt::to_string(resolve_variable_name(reader.read_pod<u32>() & 0xFFFFFF, form));
			case 0x6: return fmt::format("\"{}\"", get_string(reader.read_pod<s32>(), form));
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

		case 0x15: break;

		case 0x45: {
			auto a = reader.read_pod<Block>();
			mnemonic = fmt::format("pop.{}.{}", type_suffix(t1), type_suffix(t2));
			params = instance_name(a);
		} break;

		case 0x9D: break;

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

		case 0xC1: break;
		case 0xC2: break;

		case 0xC3: {
			mnemonic = fmt::format("pushvar.{}", type_suffix(t2));
			params = push_param(t1);
		} break;

		case 0x84: {
			mnemonic = "push.i16";
			params = fmt::to_string(main_block & 0xFFFF);
		} break;

		case 0xD9: break;

		default: {
			mnemonic = "<bad>";
			comment = "!!! This may indicate corruption";
		}

		};

		bool mnemonic_warning = !mnemonic.empty() && mnemonic[0] == '<';
		bool params_warning = !params.empty() && params[0] == '<';

		comment = fmt::format(
			"${:08x} ",
			fmt::join(std::vector(old_reader.pos, reader.pos), "'")
		) + comment;

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
