#pragma once

#include <cstddef>

constexpr bool debug_mode = true;

constexpr std::size_t max_stack_depth = 1024 * 32; // 32KiB
constexpr std::size_t max_context_depth = 256;
constexpr std::size_t max_call_depth = 256;
