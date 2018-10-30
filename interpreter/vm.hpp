#ifndef VM_HPP
#define VM_HPP

#include "../unpack/decode.hpp"
#include "../bytecode/types.hpp"

class VM
{
	Form& _form;

	std::vector<char> stack;

public:
	VM(Form& form) :
		_form{form}
	{}

	void execute(Script& script);
};

#endif // VM_HPP
