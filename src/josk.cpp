#include <josk/cli.hpp>
#include <josk/task_find_plugins.hpp>
#include <josk/task_load.hpp>
#include <josk/tasks.hpp>

#include <CLI/App.hpp>

#include <cstdio>
#include <cstdlib>
#include <expected>
#include <print>
#include <utility>
#include <vector>

#include "josk/task_preparse.hpp"

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

	std::vector<josk::task::task_preparse_records> preparse_tasks{};
	for (const auto& load_file_task : find_plugins_result.value())
	{
		auto load_file_result = josk::load_file(load_file_task);
		if (!load_file_result.has_value())
		{
			std::println(stderr, "{}", load_file_result.error());
			return EXIT_FAILURE;
		}
		if (auto& load_file_data = load_file_result.value(); !load_file_data.groups.empty())
		{
			preparse_tasks.emplace_back(std::move(load_file_data));
		}
	}

	std::vector<josk::task::task_merge_records> merge_tasks{};
	for (const auto& preparse_task : preparse_tasks)
	{
		auto preparse_result = josk::preparse_file(preparse_task);
		if (!preparse_result.has_value())
		{
			std::println(stderr, "{}", preparse_result.error());
			return EXIT_FAILURE;
		}
		if (auto& preparse_file_data = preparse_result.value(); !preparse_file_data.groups.empty())
		{
			merge_tasks.emplace_back(std::move(preparse_file_data));
		}
	}

	return !merge_tasks.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
}
