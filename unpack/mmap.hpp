#ifndef MMAP_HPP
#define MMAP_HPP

#include <string>

class ReadMappedFile
{
	FILE* _file;
	std::size_t _size;
	char* _address = nullptr;

public:
	ReadMappedFile() = default;
	ReadMappedFile(const std::string& name);

	~ReadMappedFile();

	operator bool() const;

	char* data();
	const char* data() const;
};

inline ReadMappedFile::operator bool() const
{
	return _address != nullptr;
}

inline char* ReadMappedFile::data()
{
	return _address;
}

inline const char* ReadMappedFile::data() const
{
	return _address;
}

#endif // MMAP_HPP
