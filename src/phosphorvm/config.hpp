#pragma once

#include <cstddef>

namespace debug
{
constexpr bool
	//! Override, which enables all flag checks.
	debug_everything = false,

	//! Provides a lot of information on the unpacked data.win file.
	verbose_unpack = true,

	//! Provide debug information for bytecode post-processing (reference
	//! processing, etc.).
	verbose_postprocess = true,

	//! Provide information for every C++ function <-> FunctionDefinition
	//! binding done
	verbose_bindings = true,

	//! Disassembles the bytecode program.
	disassemble = true,

	//! Prints a debug message on every single instruction in the VM.
	vm_verbose_instructions = false,

	//! Makes operation on the main VM stack verbose.
	vm_verbose_stack = false,

	//! Prints a debug message when entering/leaving any function.
	vm_verbose_calls = false,

	//! Initializes stacks with a recognizable pattern to help detect
	//! uninitialized stack usage.
	vm_debug_stack = true,

	//! Trades performance for safety. This includes:
	//! - Turning maybe_unreachable() calls from __builtin_unreachable to an
	//!   exception.
	//! - Checking the boundaries for the block reader.
	//! - Checking stack limits (main stack, call stack, ...).
	vm_safer = false;
}

constexpr bool check(const bool flag)
{
	return debug::debug_everything || flag;
}

constexpr std::size_t
	max_stack_depth = 1024 * 32, // 32KiB
	max_context_depth = 16,
	max_call_depth = 256;
