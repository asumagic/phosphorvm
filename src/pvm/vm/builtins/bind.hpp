#pragma once

#include "pvm/unpack/chunk/function.hpp"
#include "pvm/vm/builtin.hpp"
#include "pvm/vm/builtins/bindwrapper.hpp"
#include <algorithm>
#include <fmt/core.h>
#include <string_view>
#include <type_traits>

template<auto BindFunc>
void bind(Func& func_chunk, std::string_view name)
{
	auto func_it = std::find_if(
		func_chunk.definitions.begin(),
		func_chunk.definitions.end(),
		[name](FunctionDefinition& def) { return def.name == name; });

	if constexpr (check(debug::verbose_postprocess))
	{
		fmt::print(
			"Binding function '{}', id {}",
			name,
			std::distance(func_chunk.definitions.begin(), func_it));
	}

	if constexpr (std::is_same_v<decltype(BindFunc), GenericBuiltin*>)
	{
		func_it->associated_builtin = BindFunc;
	}
	else
	{
		func_it->associated_builtin = std_wrapper<BindFunc>;
	}
}
