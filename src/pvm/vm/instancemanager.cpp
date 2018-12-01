#include "pvm/vm/instancemanager.hpp"

#include "pvm/bc/enums.hpp"

InstanceManager::InstanceManager() :
	_global{_instances.emplace(s32(InstType::global), Instance{}).first->second}
{}

Instance& InstanceManager::global()
{
	return _global;
}
