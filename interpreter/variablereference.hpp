#pragma once

#include "../bytecode/enums.hpp"
#include "../bytecode/types.hpp"
#include "vm.hpp"
#include <optional>

template<class T>
class VariableReference
{
	InstType _inst_type;
	s32 _var_id;

	//! Can be used in two ways:
	//! - Storing the value for a temporary VariableReference for reading
	//! - Storing the value to be stored using VM::write_variable
	std::optional<T> _stored_value = {};

public:
	using value_type = T;

	VariableReference(InstType inst_type = InstType::stack_top_or_global, s32 var_id = 0);

	//! Reads the value into 'stored_value'. This can alter the VM state
	//! because it will pop stack variables.
	T read(VM& vm);
};

template<class T>
VariableReference<T>::VariableReference(InstType inst_type, s32 var_id) :
	_inst_type{inst_type},
	_var_id{var_id}
{}

template<class T>
T VariableReference<T>::read(VM& vm)
{
	if (!_stored_value)
	{
		switch (_inst_type)
		{
		case InstType::stack_top_or_global:
			// At this time the only part of the stack variable that is left is
			// the actual value, *including padding*, which we have to care about!
			vm.stack.skip(sizeof(s64) - sizeof(T));
			_stored_value = vm.stack.pop<T>();
			break;

		default:
			maybe_unreachable("Unimplemented pop_variable for T");
		}
	}

	return _stored_value.value();
}
