#include "pvm/interpreter/builtins/bind.hpp"

#include <fmt/core.h>
#include <fmt/color.h>
#include "pvm/config.hpp"
#include "pvm/unpack/chunk/function.hpp"

Bindings::Bindings(const Func& funcs) :
	_funcs{funcs}
{}

std::size_t Bindings::determine_function_index(std::string_view name) const
{
	auto it = std::find_if(
		_funcs.definitions.begin(),
		_funcs.definitions.end(),
		[&](const FunctionDefinition& func) {
			return func.name == name;
		}
	);

	return std::distance(_funcs.definitions.begin(), it);
}

std::size_t Bindings::push(Builtin builtin)
{
	std::size_t index = determine_function_index(builtin.name);

	if (index >= _builtins.size())
	{
		_builtins.resize(index + 1);
	}

	if (_builtins[index] == nullptr)
	{
		if constexpr (check(debug::verbose_bindings))
		{
			fmt::print(
				"Bound builtin {} as function {}\n",
				builtin.name,
				index
			);
		}

		_builtins[index] = builtin.func;

		const_cast<FunctionDefinition&>(_funcs.definitions[index]).associated_builtin = builtin.func;
	}
	else
	{
		fmt::print(
			fmt::color::orange,
			"Tried to redefine builtin '{}', ignoring\n",
			builtin.name
		);
	}

	return _builtins.size() - 1;
}

GenericBuiltin& Bindings::get(std::size_t id) const
{
	return *_builtins[id];
}
