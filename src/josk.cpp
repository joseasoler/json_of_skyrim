#include <josk/cli.hpp>
#include <josk/task_find_plugins.hpp>
#include <josk/task_load.hpp>

#include <CLI/App.hpp>

#include <cstdio>
#include <cstdlib>
#include <expected>
#include <print>

int main(const int argc, char* argv[])
{
	CLI::App app{};
	josk::cli::args arguments{};
	josk::cli::configure_cli(app, arguments);
	CLI11_PARSE(app, argc, argv);
	if (const auto validation = josk::cli::validate_arguments(arguments); !validation.has_value())
	{
		std::println(stderr, "{}", validation.error());
		return EXIT_FAILURE;
	}

	const auto find_plugins_result = josk::find_plugins(arguments);

	if (!find_plugins_result.has_value())
	{
		std::println(stderr, "{}", find_plugins_result.error());
		return EXIT_FAILURE;
	}

	const auto& test_task = find_plugins_result.value().front();

	[[maybe_unused]] const auto load_file_result = josk::load_file(test_task);
	return EXIT_SUCCESS;
}
