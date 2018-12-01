#include "pvm/vm/instancemanager.hpp"

#include "pvm/bc/enums.hpp"

InstanceManager::InstanceManager()
{
	for (InstType special_id : {InstType::global})
	{
		_instances.emplace(s32(special_id), Instance{});
	}
}

Instance& InstanceManager::global()
{
	return _instances[s32(InstType::global)];
}
