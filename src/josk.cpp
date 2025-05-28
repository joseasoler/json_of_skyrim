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
	if (const auto validation = josk::cli::validate_arguments(arguments); !validation.has_value())
	{
		std::println(stderr, "{}", validation.error());
		return EXIT_FAILURE;
	}

	auto load_order_data = josk::task::parse_load_order(arguments.load_order_path);
	if (!load_order_data.has_value())
	{
		std::println(stderr, "{}", load_order_data.error());
		return EXIT_FAILURE;
	}

	const auto find_plugins_result =
			josk::task::find_plugins(arguments.skyrim_data_path, load_order_data.value(), arguments.mods_path);

	if (!find_plugins_result.has_value())
	{
		std::println(stderr, "{}", find_plugins_result.error());
		return EXIT_FAILURE;
	}

	const auto preparse_plugins_result = josk::task::preparse_and_merge_plugins(find_plugins_result.value());
	if (!preparse_plugins_result.has_value())
	{
		std::println(stderr, "{}", preparse_plugins_result.error());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
