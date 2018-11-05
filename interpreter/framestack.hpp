#pragma once

#include "../config.hpp"
#include "frame.hpp"
#include <array>

struct FrameStack
{
	std::array<Frame, max_call_depth> frames;

	Frame& push();
	void pop();
};
