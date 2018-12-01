#pragma once

#include "pvm/bc/enums.hpp"
#include "pvm/vm/instance.hpp"
#include "pvm/vm/instancemanager.hpp"

struct Context
{
	//! Cached instance pointer. It is valid only if the inst_id refers to only
	//! one instance, which is not the case when dealing with 'all' for
	//! instance.
	Instance* cached_instance = nullptr;

	s32 inst_id;
};
