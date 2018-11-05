#pragma once

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

	std::size_t size() const;

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
