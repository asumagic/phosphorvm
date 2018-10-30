#ifndef DISASM_HPP
#define DISASM_HPP

#include "../unpack/decode.hpp"
#include <vector>

class Disassembler
{
	const Form& _form;

	const Block* block_ptr;

public:
	Disassembler(const Form& form) :
		_form{form}
	{}

	static std::string type_suffix(u32 type);
	static std::string instance_name(InstId id);
	static std::string comparator_name(u8 function);

	const std::string& get_string(s32 id);
	std::string resolve_variable_name(s32 id);
	std::string resolve_function_name(s32 id);

	void operator()(const Script& script);
};

#endif // DISASM_HPP
