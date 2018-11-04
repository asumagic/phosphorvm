#ifndef ENUMS_HPP
#define ENUMS_HPP

// TODO: might be UB to do Instr(somevalue) when somevalue is not a legal value
// of the enum. Clang emits a warning in disasm's switch because of the default
// enum case when all enum cases are covered. Applies to the other enums too.

enum class DataType
{
	f64 = 0x0,
	f32,
	i32,
	i64,
	b32,
	var,
	str,
	i16 = 0xF
};

enum class InstType
{
	stack_top_or_global = 0,
	self = -1,
	other = -2,
	all = -3,
	noone = -4,
	global = -5,
	// TODO: figure out -6
	local = -7
};

enum class VarType
{
	array = 0x00,
	stack_top = 0x80,
	normal = 0xA0,
	unknown = 0xE0
};

enum class CompFunc
{
	lt = 0x1,
	lte,
	eq,
	neq,
	gte,
	gt
};

enum class Instr
{
	opconv    = 0x07,
	opmul     = 0x08,
	opdiv     = 0x09,
	oprem     = 0x0A,
	opmod     = 0x0B,
	opadd     = 0x0C,
	opsub     = 0x0D,
	opand     = 0x0E,
	opor      = 0x0F,
	opxor     = 0x10,
	opneg     = 0x11,
	opnot     = 0x12,
	opshl     = 0x13,
	opshr     = 0x14,
	opcmp     = 0x15,
	oppop     = 0x45,
	oppushi16 = 0x84,
	opdup     = 0x86,
	opret     = 0x9C,
	opexit    = 0x9D,
	oppopz    = 0x9E,
	opb       = 0xB6,
	opbt      = 0xB7,
	opbf      = 0xB8,
	oppushenv = 0xBA,
	oppopenv  = 0xBB,
	oppushcst = 0xC0,
	oppushloc = 0xC1,
	oppushglb = 0xC2,
	oppushfst = 0xC3,
	opcall    = 0xD9,
	opbreak   = 0xFF
};

#endif // ENUMS_HPP
