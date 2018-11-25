#pragma once

#include "pvm/vm/blockreader.hpp"
#include "pvm/bc/enums.hpp"
#include "pvm/bc/types.hpp"

struct VMState
{
	BlockReader* reader;

	const Block block;
	const Instr opcode;
	const DataType t1, t2;

	VMState(BlockReader& reader);
};

inline VMState::VMState(BlockReader& reader) :
	reader{&reader},
	block{reader.current_block()},
    opcode{Instr(block >> 24u)},
    t1{DataType((block >> 16u) & 0xFu)},
    t2{DataType((block >> 20u) & 0xFu)}
{}
