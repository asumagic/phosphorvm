#pragma once

#include "pvm/unpack/chunk/common.hpp"

struct VariableDefinition
{
	SpecialVar special_var = SpecialVar::none;

	std::string name;

	//! This is a 16-bit value within the bytecode but seems to be 32-bit here for a unknown reason.
	//! This should be tested with other values (especially positive) to see if the upper bits become
	//! zero of if they stay 0xFFFF (in which case it would be another value, or padding)
	s32 instance_type;

	//! Seems to be some sort of id, within an instance perhaps?
	u32 unknown;

	//! Amount of occurrences.
	u32 occurrences;

	//! File offset to the first instruction referring to this variable. We don't really need it here.
	u32 first_address;

	void debug_print() const
	{
		fmt::print(
			"\tVariable {:20}, instance type {} (unknown={:04x}).\n",
			name,
			instance_type,
			unknown
		);
	}
};

/*
	The format for this is somewhat awkward - it doesn't look like a List<VariableDefinition>.
	I suppose GM:S 1.4 is bytecode version 0xF (I haven't found where that actual version number is...)
	However:
	- https://gitlab.com/snippets/14943 : No mention of a change for this (but it does mention
	  the VariableDefinition change)
	- https://pcy.ulyssis.be/undertale/unpacking-corrected indicates that this should be a
	  List<VariableDefintion> - but it isn't.
	It doesn't match the older layout of VARI either, but it indeed seems to use a simple array.

	Here is how it looks:
	Vari
	|- unknown1 : UInt32
	|- unknown2 : Uint32 // seems to mirror the value of unknown1
	|- unknown3 : Uint32 // 0x00000001
	|- Definition : VariableDefinition[Count]
*/
struct Vari : Chunk
{
	std::vector<VariableDefinition> definitions;

	void debug_print() const
	{
		for (auto& def : definitions)
		{
			def.debug_print();
		}
	}
};

inline void user_reader(VariableDefinition& def, Reader& reader)
{
	reader
		>> string_reference(def.name)
		>> def.instance_type
		>> def.unknown
		>> def.occurrences
		>> def.first_address;
}

inline void user_reader(Vari& vari, Reader& reader)
{
	reader
		>> skip(3 * 4)
		>> container(vari.definitions, (vari.header.length - (3 * 4)) / 20);
}
