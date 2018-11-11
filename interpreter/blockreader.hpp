#pragma once

#include "../unpack/chunk/code.hpp"
#include "../bytecode/types.hpp"
#include "../config.hpp"
#include <stdexcept>

class BlockReader
{
	const Block* _main_block;
	std::size_t _offset;
	std::size_t _end;

public:
	BlockReader(const Script& script);

	const Block& current_block() const;

	const Block& next_block();
	const Block& relative_jump(s32 offset);

	std::size_t offset() const;
};

BlockReader::BlockReader(const Script& script) :
    _main_block{script.data.data()},
    _offset{0},
    _end{script.data.size()}
{}

const Block& BlockReader::current_block() const
{
	return _main_block[_offset];
}

const Block& BlockReader::next_block()
{
	++_offset;

	if constexpr (debug_mode)
	{
		if (_offset >= _end)
		{
			throw std::logic_error{"VM unexpectedly reached end of program"};
		}
	}

	return current_block();
}

const Block& BlockReader::relative_jump(s32 offset)
{
	_offset += offset;

	return current_block();
}

std::size_t BlockReader::offset() const
{
	return _offset;
}
