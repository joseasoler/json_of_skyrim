#include <josk/cli.hpp>
#include <josk/tasks.hpp>

#include <CLI/App.hpp>

#include <cstdio>
#include <cstdlib>
#include <expected>
#include <print>

int main(const int argc, char* argv[])
{
	CLI::App app{};
	josk::cli::arguments_t arguments{};
	josk::cli::configure_cli(app, arguments);
	CLI11_PARSE(app, argc, argv);

	const auto tasks_result = josk::cli::validate_arguments(arguments)
																.and_then(josk::task::initialize_parsing)
																.and_then(josk::task::parse_load_order)
																.and_then(josk::task::find_plugins)
																.and_then(josk::task::preparse_plugins);

	if (!tasks_result.has_value())
	{
		std::println(stderr, "{}", tasks_result.error());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
