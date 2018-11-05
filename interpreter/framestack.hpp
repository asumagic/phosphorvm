#pragma once

#include "../config.hpp"
#include "frame.hpp"
#include <array>

class FrameStack
{
	std::size_t offset = 0;

public:
	std::array<Frame, max_call_depth> frames = {};

	Frame& push();
	void pop();
	Frame& top();
};

inline Frame& FrameStack::top()
{
	return frames[offset];
}
