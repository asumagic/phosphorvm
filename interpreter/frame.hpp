#pragma once

#include <unordered_map>
#include "variable.hpp"

struct Frame
{
	std::unordered_map<std::int32_t, Variable> locals;
};
