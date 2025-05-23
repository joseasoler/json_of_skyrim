#include <josk/cli.hpp>

#include <CLI/App.hpp>

int main(const int argc, char* argv[])
{
	CLI::App app{};
	josk::cli::args arguments{};
	josk::cli::configure_cli(app, arguments);
	CLI11_PARSE(app, argc, argv);

	return 0;
}
