#ifndef WINFILE_HPP
#define WINFILE_HPP

#include <cstdint>
#include <fstream>
#include <string>
#include <type_traits>

class WinFileSection
{
	std::istream* _stream;
	std::streamoff _pos;

public:
	WinFileSection(std::istream& stream, std::streamoff pos = {});
	WinFileSection(const WinFileSection&) = default;
	WinFileSection& operator=(const WinFileSection&) = default;

	WinFileSection& operator()(char* target, std::size_t bytes);

	template<class T>
	WinFileSection& operator()(T& target);

	WinFileSection& operator()(std::string& str, size_t count);

	WinFileSection read_at(std::streamoff off);

	void skip(std::streamoff count);
};

template<class T>
WinFileSection& WinFileSection::operator()(T& target)
{
	if constexpr (std::is_pod_v<T>)
	{
		return (*this)(reinterpret_cast<char*>(&target), sizeof(T));
	}
	else
	{
		// TODO: runtime error
		throw "Bad type";
	}
}

#endif // WINFILE_HPP
