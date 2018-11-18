#pragma once

#include "../bytecode/enums.hpp"
#include "../bytecode/types.hpp"
#include "../util/errormanagement.hpp"
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

	void set_read_value(const T& value);
	T& get_read_value();
};

template<class T>
VariableReference<T>::VariableReference(InstType inst_type, s32 var_id) :
	_inst_type{inst_type},
	_var_id{var_id}
{}

template<class T>
void VariableReference<T>::set_read_value(const T& value)
{
	_stored_value = value;
}

template<class T>
T& VariableReference<T>::get_read_value()
{
	if (!_stored_value)
	{
		maybe_unreachable("Tried to get_read_value, but never set that value");
	}

	return _stored_value.value();
}
