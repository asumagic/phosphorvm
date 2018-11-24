#pragma once

#include <vector>
#include "../builtin.hpp"

struct Func;
struct FunctionDefinition;

class Bindings
{
	const Func& _funcs;
	std::vector<GenericBuiltin*> _builtins;

	std::size_t determine_function_index(std::string_view name) const;

public:
	Bindings(const Func& funcs);

	std::size_t push(Builtin builtin);
	GenericBuiltin& get(std::size_t id) const;
};
