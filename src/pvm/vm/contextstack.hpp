#pragma once

#include <array>
#include "pvm/config.hpp"
#include "pvm/vm/context.hpp"

// TODO: should this be merged with framestack if there is more context to keep
// than just the instance id etc?
struct ContextStack
{
	std::array<Context, max_context_depth> contexts;

	[[nodiscard]] Context& push();
	void pop();
};
