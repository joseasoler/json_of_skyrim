#include <josk/cli.hpp>

#include <CLI/App.hpp>

int main(const int argc, char* argv[])
{
	/*
	if (argc == 2)
	{
		// ToDo hardcoded for now.
		using josk::tes::record_type;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
	*/
	CLI::App app{};
	josk::cli::args arguments{};
	josk::cli::configure_cli(app, arguments);
	CLI11_PARSE(app, argc, argv);

	return 0;
}
