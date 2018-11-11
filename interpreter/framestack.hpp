#pragma once

#include "../config.hpp"
#include "frame.hpp"
#include <array>
#include <stdexcept>

struct FrameStack
{
	std::size_t offset = 0;

	std::array<Frame, max_call_depth> frames;

	Frame& push();
	void pop();
	Frame& top();
};

inline Frame& FrameStack::push()
{
	if constexpr (debug_mode)
	{
		if (offset >= frames.size())
		{
			throw std::runtime_error{"Frame stack limit reached"};
		}
	}

	new(&frames[++offset]) Frame{};
	return top();
}

inline void FrameStack::pop()
{
	if constexpr (debug_mode)
	{
		if (offset == 0)
		{
			throw std::logic_error{"Tried to pop last stack frame"};
		}
	}

	frames[offset--].~Frame();
}

inline Frame& FrameStack::top()
{
	return frames[offset];
}
