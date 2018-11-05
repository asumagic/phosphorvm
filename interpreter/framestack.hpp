#pragma once

#include "../config.hpp"
#include "frame.hpp"

struct FrameStack
{
	std::array<Frame, max_call_depth> frames;

	Frame& push();
	void pop();
};
