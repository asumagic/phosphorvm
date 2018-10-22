#ifndef HELPER_HPP
#define HELPER_HPP

#include <algorithm>
#include <cstring>
#include <string>

struct Reader
{
	const char* pos;

	void skip(std::size_t n)
	{
		pos += n;
	}

	template<class T>
	T read_pod()
	{
		T ret;
		std::memcpy(&ret, pos, sizeof(T));
		pos += sizeof(T);
		return ret;
	}

	std::string read_string()
	{
		std::string ret{pos};
		pos += ret.size();
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

	std::ptrdiff_t distance_with(Reader other)
	{
		return pos - other.pos;
	}
};

#endif // HELPER_HPP
