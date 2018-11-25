#pragma once

#include <stdexcept>
#include "pvm/bc/types.hpp"
#include "pvm/config.hpp"
#include "pvm/unpack/chunk/code.hpp"
#include "pvm/util/errormanagement.hpp"

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

	bool out_of_bounds() const;
};

inline BlockReader::BlockReader(const Script& script) :
	_main_block{script.data.data()},
	_offset{0},
	_end{script.data.size()}
{}

inline const Block& BlockReader::current_block() const
{
	return _main_block[_offset];
}

inline const Block& BlockReader::next_block()
{
	// TODO: fix this check
	if (out_of_bounds())
	{
		maybe_unreachable("VM unexpectedly reached end of program");
	}

	++_offset;

	return current_block();
}

inline const Block& BlockReader::relative_jump(s32 offset)
{
	_offset += offset;

	return current_block();
}

inline std::size_t BlockReader::offset() const
{
	return _offset;
}

inline bool BlockReader::out_of_bounds() const
{
	return _offset >= _end;
}
