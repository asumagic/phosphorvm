#pragma once

#include "blockreader.hpp"
#include "../bytecode/enums.hpp"
#include "../bytecode/types.hpp"

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
	opcode{Instr(block >> 24)},
	t1{DataType((block >> 16) & 0xF)},
	t2{DataType((block >> 20) & 0xF)}
{}
