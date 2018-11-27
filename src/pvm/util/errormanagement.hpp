#pragma once

#include "pvm/config.hpp"
#include "pvm/util/compilersupport.hpp"
#include <stdexcept>

//! Call to designate a normally unreachable spot.
//! With debug_mode set, throws an exception, otherwise purposefully UB.
[[noreturn]] FORCE_INLINE inline void
maybe_unreachable(const char* error = "Unreachable VM state")
{
	if constexpr (check(debug::vm_safer))
	{
		throw std::runtime_error{error};
	}

	UNREACHABLE
}
