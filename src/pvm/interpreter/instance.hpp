#pragma once

#include "pvm/interpreter/variable.hpp"
#include <unordered_map>

class Instance
{
	std::unordered_map<s32, Variable> var;
};
