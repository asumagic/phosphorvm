#pragma once

#include <vector>
#include "pvm/unpack/decode.hpp"

struct DisassembledInstruction
{
	std::string location, mnemonic = "<unknown>", params, comment;
	const Block *begin_block, *end_block;

	[[nodiscard]] std::string as_plain_string() const;
};

class Disassembler
{
	const Form& _form;

public:
	Disassembler(const Form& form) :
		_form{form}
	{}

	[[nodiscard]] static std::string type_suffix(u32 type);
	[[nodiscard]] static std::string instance_name(InstId id);
	[[nodiscard]] static std::string comparator_name(u8 function);

	[[nodiscard]] std::string get_string(s32 id);
	[[nodiscard]] std::string resolve_variable_name(s32 id);
	[[nodiscard]] std::string resolve_function_name(s32 id);

	[[nodiscard]] DisassembledInstruction disassemble_block(const Block* block, const Script* script = nullptr);

	void operator()(const Script& script);
};
