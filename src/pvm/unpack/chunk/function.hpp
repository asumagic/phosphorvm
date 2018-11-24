#pragma once

#include "pvm/interpreter/builtin.hpp"
#include "pvm/unpack/chunk/code.hpp"
#include "pvm/unpack/chunk/common.hpp"

// See VariableDefinition for an explanation on some of the fields below
struct FunctionDefinition
{
	std::string name;
	u32 occurrences;
	u32 first_address;

	bool is_builtin;
	Script* associated_script;
	GenericBuiltin* associated_builtin;

	void debug_print() const
	{
		fmt::print("\tFunction {}\n", name);
	}
};

struct Func : Chunk
{
	std::vector<FunctionDefinition> definitions;

	void debug_print() const
	{
		for (auto& def : definitions)
		{
			def.debug_print();
		}
	}
};

inline void user_reader(FunctionDefinition& def, Reader& reader)
{
	reader
	    >> string_reference(def.name)
	    >> def.occurrences
	    >> def.first_address;
}

inline void user_reader(Func& func, Reader& reader)
{
	u32 count;
	reader
	    >> count
	    >> container(func.definitions, count);
}
