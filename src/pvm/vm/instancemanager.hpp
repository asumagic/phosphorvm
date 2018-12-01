#pragma once

#include "pvm/bc/types.hpp"
#include "pvm/vm/instance.hpp"
#include <vector>

class InstanceManager
{
	std::unordered_map<InstId, Instance> _instances;
	Instance&                            _global;

	public:
	InstanceManager();

	Instance& global();
};
