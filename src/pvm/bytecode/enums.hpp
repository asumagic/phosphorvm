#pragma once

#include "pvm/bytecode/types.hpp"

// TODO: might be UB to do Instr(somevalue) when somevalue is not a legal value
// of the enum. Clang emits a warning in disasm's switch because of the default
// enum case when all enum cases are covered. Applies to the other enums too.

enum class DataType : s8
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

enum class InstType : s32
{
	stack_top_or_global = 0,
	self = -1,
	other = -2,
	all = -3,
	noone = -4,
	global = -5,
	special = -6,
	local = -7
};

enum class VarType : u8
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

// Warning: SpecialVar lookup using string keys can be performed using
// "bytecode/names.hpp", make sure you keep both in-sync!
enum class SpecialVar
{
	// Arguments should always begin at 0; VM::push_special relies on it
	argument0 = 0,
	argument1,
	argument2,
	argument3,
	argument4,
	argument5,
	argument6,
	argument7,
	argument8,
	argument9,
	argument10,
	argument11,
	argument12,
	argument13,
	argument14,
	argument15,

	argument_count,

	//! Current instance id
	id,

	none
};

enum class Instr : u8
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
	oppushspc = 0xC3, //! See ODDITIES.md for details about $C3
	opcall    = 0xD9,
	opbreak   = 0xFF
};
