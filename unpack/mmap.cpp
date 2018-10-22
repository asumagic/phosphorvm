#include "mmap.hpp"
#include <errno.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <stdexcept>
#include <unistd.h>

ReadMappedFile::ReadMappedFile(const std::string& name)
{
	_file = fopen(name.c_str(), "r");
	if (!_file)
	{
		throw std::runtime_error{fmt::format("File '{}' could not be opened: {}", name, strerror(errno))};
	}

	fseek(_file, 0, SEEK_END);
	_size = ftell(_file);

	_address = static_cast<char*>(mmap(0, _size, PROT_READ, MAP_PRIVATE, fileno(_file), 0));
	if (_address == MAP_FAILED)
	{
		throw std::runtime_error{fmt::format("mmap() for '{}' failed: {}", name, strerror(errno))};
	}
}

ReadMappedFile::~ReadMappedFile()
{
	if (_address != nullptr)
	{
		if (munmap(_address, _size) == -1)
		{
			fmt::print("munmap() failed... silently failing: {}\n", strerror(errno));
		}

		if (!fclose(_file))
		{
			fmt::print("close() failed... silently failing: {}\n", strerror(errno));
		}
	}
}
