#include <fmt/core.h>
#include "unpack/decode.hpp"

int main()
{
	std::ifstream file{"data.win"};

	if (!file)
	{
		fmt::print("Missing 'data.win' in working directory! Failing.\n");
		return 1;
	}

	WinFileSection main_section{file};
	Form main_form;
	main_form.read(main_section);
}
