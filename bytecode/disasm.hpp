#ifndef DISASM_HPP
#define DISASM_HPP

#include "../unpack/decode.hpp"
#include <vector>

void print_disassembly(Form& form, const std::vector<char>& program);

#endif // DISASM_HPP
