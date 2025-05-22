#include <josk/tes_format.hpp>
#include <josk/tes_reader.hpp>

#include <iostream>
#include <unordered_set>

int main(const int argc, char* argv[])
{
	if (argc == 2)
	{
		// ToDo hardcoded for now.
		using josk::tes::record_type;
		const auto result = josk::tes::read_file(argv[1], std::unordered_set{record_type::avif});
		if (result.has_value())
		{
			std::cout << static_cast<int>(result->begin()->first) << ", " << result->begin()->second.size() << '\n';
		}
		else
		{
			std::cerr << result.error() << '\n';
		}
	}
	return 0;
}
