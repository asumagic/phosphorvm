#pragma once

#include "../config.hpp"
#include "frame.hpp"
#include <array>

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
	new(&frames[++offset]) Frame{};
	return top();
}

inline void FrameStack::pop()
{
	frames[offset--].~Frame();
}

inline Frame& FrameStack::top()
{
	return frames[offset];
}
