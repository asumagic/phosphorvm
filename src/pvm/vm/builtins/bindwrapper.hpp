#pragma once

#include <tuple>
#include <type_traits>
#include "pvm/bc/types.hpp"
#include "pvm/vm/traits/funcinfo.hpp"
#include "pvm/vm/vm.hpp"

template<auto Func>
inline auto std_wrapper = [](VM& vm, const Frame& frame) {
	using FuncSignature = std::remove_pointer_t<decltype(Func)>;
	using FuncInfo = func_info<FuncSignature>;

	if (frame.argument_count != std::tuple_size_v<typename FuncInfo::args_type>)
	{
		exit(0);
	}

	auto fill_params = [&](auto... params)
	{
		return std::tuple{vm.read_variable_parameter<decltype(params)>()...};
	};

	auto ret = std::apply(Func, std::apply(fill_params, typename FuncInfo::args_type{}));

	if constexpr (std::is_same_v<typename FuncInfo::return_type, void>)
	{
		vm.push_stack_variable(s32(0));
	}
	else
	{
		vm.push_stack_variable(ret);
	}
};
