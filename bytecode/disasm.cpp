#include "disasm.hpp"
#include <fmt/core.h>

void print_disassembly(Form& form, const std::vector<char>& program)
{
	fmt::print("Disassembly: {} bytes (~{} KiB)\n", program.size(), program.size() / 1024);

	std::int32_t offset = 0;

	while (std::size_t(offset) < program.size())
	{
		fmt::print("{:0x8}: {:0x2}\n", offset, program[offset]);
	}
}
