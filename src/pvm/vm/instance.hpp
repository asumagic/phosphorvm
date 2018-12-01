#pragma once

#include "pvm/bc/types.hpp"
#include "pvm/vm/variable.hpp"
#include <unordered_map>

class Instance
{
	std::unordered_map<s32, Variable> _variables;

	public:
	Variable& variable(VarId id);
};

inline Variable& Instance::variable(VarId id)
{
	return _variables[id];
}
