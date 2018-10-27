#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include "common.hpp"

struct VariableDefinition
{
	std::string name;

	//! This is a 16-bit value within the bytecode but seems to be 32-bit here for a unknown reason.
	//! This should be tested with other values (especially positive) to see if the upper bits become
	//! zero of if they stay 0xFFFF (in which case it would be another value, or padding)
	s16 instance_type;

	//! Seems to be some sort of id, within an instance perhaps?
	u32 unknown;

	u32 occurrences;

	//! Address of the first occurence of this variable.
	//! It appears that when there is no occurence, this address will be 0xFFFFFFFF.
	u32 first_address;
};

/*
	The format for this is somewhat awkward - it doesn't look like a List<VariableDefinition>.
	I suppose GM:S 1.4 is bytecode version 0xF (I haven't found where that actual version number is...)
	However:
	- https://gitlab.com/snippets/14943 : No mention of a change for this change (but it does mention
	  the VariableDefinition change)
	- https://pcy.ulyssis.be/undertale/unpacking-corrected indicates that this should be a
	  List<VariableDefintion> - but it isn't.
	It doesn't match the older layout of VARI either, but it indeed seems to use a simple array.

	Here is how it looks:
	Vari
	|- Count : UInt32
	|- unknown : Uint32 // seems to mirror the value of Count
	|- unknown : Uint32 // 0x00000001
	|- Definition : VariableDefinition[Count]
*/
struct Vari
{
	std::vector<VariableDefinition> definitions;
};

inline void read(VariableDefinition& def, Reader& reader)
{
	def.name = reader.read_string_reference();
	def.instance_type = reader.read_pod<s32>();
	def.unknown = reader.read_pod<u32>();
	def.occurrences = reader.read_pod<u32>();
	def.first_address = reader.read_pod<u32>();

	fmt::print(
		"\tVariable {:20}, {:4} appearances, first addr {:04x}, instance type {}\n",
		def.name,
		def.occurrences,
		def.first_address,
		def.instance_type
	);
}

inline void read(Vari& vari, Reader& reader)
{
	u32 count = reader.read_pod<u32>();
	reader.skip(2 * 4); // unknown
	// first u32 seems to mirror count?
	// next byte is 1 in this data.win

	for (u32 i = 0; i < count; ++i)
	{
		read(vari.definitions.emplace_back(), reader);
	}
}

#endif // VARIABLE_HPP
