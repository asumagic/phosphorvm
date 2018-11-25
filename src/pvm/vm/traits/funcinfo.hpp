#pragma once

#include <tuple>

template<class T>
struct func_info;

template<class Ret, class... Args>
struct func_info<Ret(Args...)>
{
	using args_type = std::tuple<Args...>;
	using return_type = Ret;
};
