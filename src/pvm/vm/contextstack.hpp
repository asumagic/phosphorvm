#pragma once

#include "pvm/config.hpp"
#include "pvm/vm/context.hpp"
#include <array>

struct ContextStack
{
	std::array<Context, max_context_depth> contexts;

	[[nodiscard]] Context& push();
	void                   pop();
};
