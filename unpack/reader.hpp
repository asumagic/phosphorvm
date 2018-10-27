#ifndef HELPER_HPP
#define HELPER_HPP

#include <algorithm>
#include <cstring>
#include <string>
#include <fmt/core.h>

template<class T>
struct GenericReader
{
	const T* begin;
	const T* pos = begin;

	//! Skip n bytes. Negative offsets are allowed.
	void skip(std::ptrdiff_t n)
	{
		pos += n;
	}

	template<class Ret>
	Ret read_pod()
	{
		Ret ret{};
		std::memcpy(&ret, pos, sizeof(Ret));
		pos += sizeof(Ret) / sizeof(T);
		return ret;
	}

	template<class Ret>
	Ret read_pod_container(std::size_t count)
	{
		Ret ret;
		ret.resize(count);

		std::generate(ret.begin(), ret.end(), [&] {
			return read_pod<typename Ret::value_type>();
		});

		return ret;
	}

	std::ptrdiff_t distance_with(const GenericReader<T>& other) const
	{
		return pos - other.pos;
	}

	std::size_t offset() const
	{
		return std::size_t(pos - begin);
	}

	std::size_t bytes() const
	{
		return offset() * sizeof(T);
	}
};

struct Reader : GenericReader<char>
{
	std::string read_string_reference()
	{
		return {begin + read_pod<std::uint32_t>()};
	}

	std::string read_string(std::size_t count)
	{
		std::string ret{pos, pos + count};
		pos += count;
		return ret;
	}

	template<class T>
	void read_into(T& t)
	{
		read(t, *this);
	}
};

#endif // HELPER_HPP
