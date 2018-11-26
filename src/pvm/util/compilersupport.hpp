#pragma once

#ifdef __GNUC__
    #define FORCE_INLINE __attribute__((always_inline))
	#define UNREACHABLE __builtin_unreachable();
#else
    #define FORCE_INLINE
	#define UNREACHABLE
#endif
