#ifndef HELPER_HPP
#define HELPER_HPP

#include <algorithm>
#include <cstring>
#include <string>
#include <fmt/core.h>

template<class T>
class GenericReader
{
protected:
	using size_type = std::size_t;
	using offset_type = std::ptrdiff_t;

	// Container information
	const T *_begin;
	size_type _size;

	// Reader position
	offset_type _offset = 0;

	void sanitize_read(offset_type read_size)
	{
		if (read_size < 0 || (_offset + read_size) > offset_type(_size))
		{
			throw std::out_of_range(fmt::format("GenericReader: Read of {} is invalid from {} (max read {})", read_size, _offset, _size - _offset));
		}
	}

public:
	GenericReader(const T* begin, std::size_t size) :
		_begin{begin},
		_size{size}
	{}

	//! Initialize a GenericReader with the same boundaries as another one.
	GenericReader(const GenericReader& base_reader, offset_type offset) :
		_begin{base_reader._begin},
		_size{base_reader._size},
		_offset{offset}
	{}

	//! Skip n bytes. Negative offsets are allowed.
	void skip(std::ptrdiff_t n)
	{
		_offset += n;
	}

	void seek(std::ptrdiff_t offset)
	{
		_offset = offset;
	}

	bool done() const
	{
		return _offset >= offset_type(_size);
	}

	template<class Ret>
	Ret read_pod()
	{
		sanitize_read(sizeof(Ret) / sizeof(T));

		Ret ret{};
		std::memcpy(&ret, _begin + _offset, sizeof(Ret));
		_offset += sizeof(Ret) / sizeof(T);

		return ret;
	}

	template<class Ret>
	Ret read_pod_container(std::size_t count)
	{
		Ret ret(count, {});

		sanitize_read(count);
		std::memcpy(ret.data(), _begin + _offset, (sizeof(typename Ret::value_type) / sizeof(T)) * count);

		return ret;
	}

	std::ptrdiff_t distance_with(const GenericReader<T>& other) const
	{
		return _offset - other._offset;
	}

	std::size_t offset() const
	{
		return _offset;
	}

	std::size_t bytes() const
	{
		return _offset * sizeof(T);
	}
};

class Reader : public GenericReader<char>
{
public:
	using GenericReader<char>::GenericReader;

	std::string read_string(offset_type count)
	{
		sanitize_read(count);
		std::string ret{_begin + _offset, _begin + _offset + count};
		_offset += count;
		return ret;
	}

	std::string read_string_reference()
	{
		// TODO: sanitize
		auto location = offset_type(read_pod<std::uint32_t>());
		return {_begin + location};
	}

	template<class T>
	void read_into(T& t)
	{
		read(t, *this);
	}
};

#endif // HELPER_HPP
