#include "winfile.hpp"

#include <cassert>

WinFileSection::WinFileSection(std::istream& stream, std::streamoff pos) :
	_stream{&stream},
	_pos{pos}
{}

WinFileSection& WinFileSection::operator()(char* target, std::size_t bytes)
{
	_stream->seekg(_pos);
	_stream->read(target, bytes);
	_pos += bytes;
	return *this;
}

WinFileSection& WinFileSection::operator()(std::string& str, size_t count)
{
	str.clear();
	str.resize(count);
	return (*this)(str.data(), count);
}

WinFileSection WinFileSection::read_at(std::streamoff off)
{
	return {*_stream, off};
}

void WinFileSection::skip(std::streamoff count)
{
	_pos += count;
}
