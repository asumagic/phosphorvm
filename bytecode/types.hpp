#pragma once

#include <cstdint>

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using Block = u32;
using InstId = s16;

static_assert(sizeof(float) == 4, "Expected float to be 32-bit fp!");
static_assert(sizeof(double) == 8, "Expected double to be 64-bit fp!");
