#include "../unpack/reader.hpp"
#include "disasm.hpp"
#include <fmt/core.h>
#include <fmt/color.h>

const std::string& get_string(std::int32_t id, Form& form)
{
	return form.strg.elements[id].value;
}

std::string type_suffix(std::int8_t type)
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

std::string instance_name(std::int16_t id)
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

void print_disassembly(Form& form, const Script& script)
{
	auto& program = script.data;

	fmt::print(fmt::color::orange, "\nDisassembly of '{}': {} bytes (~{} KiB)\n", script.name, program.size(), program.size() / 1024);

	Reader reader{program.data()};

	while (reader.offset() < program.size())
	{
		auto block = reader.read_pod<std::uint32_t>();

		auto op = (block >> 24) & 0xFF;

		// Type pairs
		auto t1 = (block >> 16) & 0xF;
		auto t2 = (block >> 20) & 0xF;

		fmt::print(fmt::color::light_gray, "0x{:08x}: ", reader.offset(), block);

		std::string mnemonic = "<unimpl>", params, comment;

		auto generic_2t = [&](auto name) {
			mnemonic = fmt::format("{}.{}.{}", name, type_suffix(t1), type_suffix(t2));
		};

		auto read_pushed_value = [&](auto type) -> std::string {
			switch (type)
			{
			case 0x0: return fmt::to_string(reader.read_pod<double>());
			case 0x1: return fmt::to_string(reader.read_pod<float>());
			case 0x2: return fmt::to_string(reader.read_pod<std::uint32_t>());
			case 0x3: return fmt::to_string(reader.read_pod<std::uint64_t>());
			case 0x6: return fmt::format("\"{}\"", get_string(reader.read_pod<std::int32_t>(), form));
			case 0xF: return fmt::to_string(block & 0xffff);
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
			auto a = reader.read_pod<std::uint32_t>();
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
			params = read_pushed_value(t1);
		} break;

		case 0xC1: break;
		case 0xC2: break;

		case 0xC3: {
			mnemonic = fmt::format("pushvar.{}", type_suffix(t1));
			params = read_pushed_value(t1);
		} break;

		case 0x84: {
			mnemonic = "push.i16";
			params = fmt::to_string(block & 0xFFFF);
		} break;

		case 0xD9: break;

		default: {
			mnemonic = "<bad>";
			comment = fmt::format(
				"\n; Unknown instruction (first block 0x{:08x}, opcode 0x{:02x}) at this point!\n"
				"; Phosphor might have incorrectly disassembled a past instruction, or the file may be corrupt.\n"
				"; Further instructions in this code block may be invalid",
				block,
				op
			);
		}

		};

		bool mnemonic_warning = !mnemonic.empty() && mnemonic[0] == '<';
		bool params_warning = !params.empty() && params[0] == '<';

		if (comment.empty())
		{
			comment = fmt::format(" ; ${:08x}", block);
		}

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

		fmt::print(fmt::color::gray, "{}\n", comment);
	}
}
