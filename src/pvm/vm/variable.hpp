#pragma once

#include "pvm/bc/enums.hpp"
#include "pvm/bc/types.hpp"
#include "pvm/vm/string.hpp"

//! Type used as a placeholder in dispatcher so instructions can detect
//! variables through type information. This may be passed to the 'f' parameter
//! called by VM:hell only when ResolveVariableReferences is false.
// It doesn't even have InstType information because we may have to peek the
// stack in order to read that value, which may be the unwanted behavior.
struct VariablePlaceholder {};

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

	constexpr static std::size_t stack_variable_size =
		sizeof(VarType) + sizeof(s64);
};
