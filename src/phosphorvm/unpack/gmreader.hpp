#pragma once

#include "../bytecode/types.hpp"
#include "reader.hpp"
#include <vector>

constexpr auto string_ascii = [] (auto& target) {
	return [&target] (auto& reader) {
		for (;;)
		{
			char c = reader();

			if (c == '\0')
			{
				break;
			}

			target.push_back(c);
		}
	};
};

constexpr auto string_reference = [] (auto& target) {
	return [&target] (auto& reader) {
		s32 offset;
		reader >> offset;

		if (offset < 0)
		{
			throw std::out_of_range{"String reference is at a negative index"};
		}

		Reader far_reader = reader;
		far_reader.seek(offset);
		//far_reader.unsafe_mode();
		far_reader >> string_ascii(target);
	};
};
