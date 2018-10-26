#ifndef HELPER_HPP
#define HELPER_HPP

#include <algorithm>
#include <cstring>
#include <string>
#include <fmt/core.h>

template<class Ptr>
struct GenericReader
{
	Ptr begin;
	Ptr pos = begin;

	//! Skip n bytes. Negative offsets are allowed.
	void skip(std::ptrdiff_t n)
	{
		pos += n;
	}

	template<class T>
	T read_pod()
	{
		T ret{};
		std::memcpy(&ret, pos, sizeof(T) / sizeof(Ptr));
		pos += sizeof(T) / sizeof(Ptr);
		return ret;
	}

	template<class C, class T>
	C read_pod_container(std::size_t count)
	{
		C ret;
		ret.resize(count);

		std::generate(ret.begin(), ret.end(), [&] {
			return read_pod<T>();
		});

		return ret;
	}

	std::ptrdiff_t distance_with(const GenericReader<Ptr>& other) const
	{
		return pos - other.pos;
	}

	std::size_t offset() const
	{
		return std::size_t(pos - begin);
	}
};

struct Reader : GenericReader<const char*>
{
	std::string read_string_reference()
	{
		auto address = read_pod<std::int32_t>();
		std::string ret{begin + address};
		return ret;
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
