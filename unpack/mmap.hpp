#ifndef MMAP_HPP
#define MMAP_HPP

#include <string>

class ReadMappedFile
{
	FILE* _file;
	long _size;
	char* _address = nullptr;

public:
	ReadMappedFile() = default;
	ReadMappedFile(const std::string& name);

	~ReadMappedFile();

	operator bool() const;

	char* data();
};

inline ReadMappedFile::operator bool() const
{
	return _address != nullptr;
}

inline char* ReadMappedFile::data()
{
	return _address;
}

#endif // MMAP_HPP
