#pragma once

#include <algorithm>
#include <fmt/core.h>
#include <fmt/ostream.h>

struct Reader;

struct ReadWrapper
{
	Reader& reader;

	template<class T>
	operator T()
	{
		T ret;
		reader >> ret;
		return ret;
	}
};

struct Reader
{
	const char *begin, *end, *pos;

	Reader(const char* p_begin, const char* p_end) :
		begin{p_begin},
		end{p_end},
		pos{p_begin}
	{}

	void sanitize_read(std::ptrdiff_t bytes)
	{
		if (begin && std::distance(pos, end) < bytes)
		{
			throw std::out_of_range{
				fmt::format(
					"Reader<T>: tried to read {} bytes but {} bytes left.\n"
					"begin={}, end={}, pos={}",
					bytes,
					std::distance(pos, end),
					fmt::ptr(begin),
					fmt::ptr(end),
					fmt::ptr(pos)
				)
			};
		}
		else if (pos < begin)
		{
			throw std::out_of_range{
				fmt::format(
					"Reader<T>: corrupted pos is before begin.\n"
					"begin={}, end={}, pos={}",
					fmt::ptr(begin),
					fmt::ptr(end),
					fmt::ptr(pos)
				)
			};
		}
	}

	void unsafe_mode()
	{
		begin = end = nullptr;
	}

	void seek(std::size_t offset)
	{
		pos = begin + offset;
	}

	ReadWrapper operator()()
	{
		return {*this};
	}
};

template<class T>
Reader& operator>>(Reader& reader, T& target)
{
	if constexpr (std::is_fundamental_v<T>)
	{
		reader.sanitize_read(sizeof(T));
		std::memcpy(&target, reader.pos, sizeof(T));
		reader.pos += sizeof(T);

		return reader;
	}
	else
	{
		user_reader(target, reader);
		return reader;
	}
}

//! Overload of stream reading for POD only
template<class T>
Reader& operator>>(Reader& reader, T&& target)
{
	target(reader);
	return reader;
}

constexpr auto skip = [] (std::size_t count) {
	return [count] (auto& reader) {
		reader.seek(std::distance(reader.begin, reader.pos) + count);
	};
};

constexpr auto container = [] (auto& target, std::size_t count) {
	return [&target, count] (auto& reader) {
		for (std::size_t i = 0; i < count; ++i)
		{
			target.push_back(reader());
		}
	};
};
