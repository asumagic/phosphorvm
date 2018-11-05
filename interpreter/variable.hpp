#pragma once

#include "../bytecode/enums.hpp"
#include "../bytecode/types.hpp"

//! Type used as a placeholder in hell so instructions can detect variables
//! through type information. This may be passed to the 'f' parameter when
//! called by VM:hell only when ResolveVariableReferences is false.
// It doesn't even have InstType information because we may have to peek the
// stack in order to read that value, which may be the unwanted behavior.
// pop<VariablePlaceholder> should successfully skip the value, however.
struct VariablePlaceholder {};

struct StringReference {};

struct Variable
{
	DataType type;

	union TypeUnion
	{
		f64 vf64;
		f32 vf32;
		s64 vs64;
		s32 vs32;
		s16 vs16;
		bool vb32;
		StringReference vstr;
	};
};

template<class T>
struct VariableReference
{
	using value_type = T;
};
