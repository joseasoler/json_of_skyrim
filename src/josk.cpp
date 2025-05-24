#include <josk/cli.hpp>
#include <josk/task_load.hpp>
#include <josk/task_load_order.hpp>
#include <josk/tasks.hpp>

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

	auto load_order_result = josk::parse_load_order(arguments.load_order_path);

	if (!load_order_result.has_value())
	{
		std::println(stderr, "{}", load_order_result.error());
		return EXIT_FAILURE;
	}

	constexpr auto* skyrim_filename = "Skyrim.esm";
	if (!load_order_result->contains(skyrim_filename))
	{
		std::println(stderr, "Could not load skyrim file.");
		return EXIT_FAILURE;
	}

	josk::task::task_load_file load_task{};
	load_task.priority = load_order_result.value()[skyrim_filename];
	load_task.path = arguments.skyrim_data_path / skyrim_filename;

	[[maybe_unused]] const auto load_file_result = josk::load_file(load_task);
	return EXIT_SUCCESS;
}
