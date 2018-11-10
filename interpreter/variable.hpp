#pragma once

#include "../bytecode/enums.hpp"
#include "../bytecode/types.hpp"

//! Type used as a placeholder in dispatcher so instructions can detect
//! variables through type information. This may be passed to the 'f' parameter
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

	constexpr static std::size_t stack_variable_size =
		sizeof(InstType) + sizeof(VarType) + sizeof(s64);
};

template<class T>
struct VariableReference
{
	using value_type = T;

	InstType inst_type;
	DataType data_type;
};
