#include "mmap.hpp"
#include <cerrno>
#include <fcntl.h>
#include <fmt/core.h>
#include <sys/mman.h>
#include <stdexcept>
#include <unistd.h>

ReadMappedFile::ReadMappedFile(const std::string& name)
{
	_file = fopen(name.c_str(), "r");
	if (_file == nullptr)
	{
		throw std::runtime_error{fmt::format("File '{}' could not be opened: {}", name, strerror(errno))};
	}

	fseek(_file, 0, SEEK_END);
	_size = ftell(_file);

	_address = static_cast<char*>(mmap(nullptr, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(_file), 0));
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

		if (fclose(_file) == EOF)
		{
			fmt::print("fclose() failed... silently failing: {}\n", strerror(errno));
		}
	}
}

std::size_t ReadMappedFile::size() const
{
	return std::size_t(_size);
}
