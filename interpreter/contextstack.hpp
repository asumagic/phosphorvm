#ifndef CONTEXTSTACK_HPP
#define CONTEXTSTACK_HPP

#include <array>
#include "../config.hpp"
#include "context.hpp"

// TODO: should this be merged with framestack if there is more context to keep
// than just the instance id etc?
struct ContextStack
{
	std::array<Context, max_context_depth> contexts;

	Context& push();
	void pop();
};

#endif // CONTEXTSTACK_HPP
