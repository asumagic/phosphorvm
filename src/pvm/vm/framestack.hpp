#pragma once

#include "pvm/config.hpp"
#include "pvm/vm/frame.hpp"
#include <array>
#include <stdexcept>

struct FrameStack
{
	std::size_t offset = 0;

	std::array<Frame, max_call_depth> frames;

	[[nodiscard]] Frame& push();
	void pop();
	[[nodiscard]] Frame& top();
	[[nodiscard]] const Frame& top() const;
};

inline Frame& FrameStack::push()
{
	if constexpr (check(debug::vm_safer))
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
	if constexpr (check(debug::vm_safer))
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

inline const Frame& FrameStack::top() const
{
	return frames[offset];
}
