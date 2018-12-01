#pragma once

#include "pvm/bc/enums.hpp"
#include "pvm/vm/instance.hpp"
#include "pvm/vm/instancemanager.hpp"

struct Context
{
	Instance* cached_instance = nullptr;
	s32       inst_id;
};
