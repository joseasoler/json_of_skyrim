#include <josk/cli.hpp>
#include <josk/task_find_plugins.hpp>
#include <josk/tasks.hpp>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{

namespace fs = std::filesystem;

/** Maps each load order filename with its priority. */
using load_order_data = std::unordered_map<std::string, josk::task::priority_t>;

std::expected<load_order_data, std::string> parse_load_order(const std::filesystem::path& load_order_path)
{
	std::ifstream input(load_order_path, std::ios::in);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not load order file {}.", load_order_path.generic_string()));
	}

	load_order_data data{};
	josk::task::priority_t priority{};
	for (std::string line; std::getline(input, line);)
	{
		if (line.front() == '#')
		{
			continue;
		}
		data[std::move(line)] = ++priority;
	}
	return data;
}

void check_add_file(fs::path path, std::vector<josk::task::task_load_file>& result, load_order_data& load_order)
{
	if (!fs::is_regular_file(path))
	{
		return;
	}

	const auto itr = load_order.find(path.filename().string());
	if (itr == load_order.end())
	{
		return;
	}

	auto& task = result.emplace_back();
	task.priority = itr->second;
	task.filename = itr->first;
	task.path = std::move(path);
	load_order.erase(itr);
}

std::string report_missing_plugins(const load_order_data& load_order_data)
{
	std::size_t missing_plugins_text_size{};
	std::vector<std::string_view> missing_plugins_names{};
	for (const auto& plugin_name : load_order_data | std::views::keys)
	{
		missing_plugins_names.emplace_back(plugin_name);
		// Each plugin will have a EOL character.
		missing_plugins_text_size += plugin_name.size() + 1U;
	}
	std::ranges::sort(missing_plugins_names);
	constexpr std::string_view error_prefix{"Could not find the following plugins: \n"};
	std::string report{};
	report.resize(error_prefix.size() + missing_plugins_text_size);
	auto report_itr = std::ranges::copy(error_prefix, report.begin()).out;
	for (const auto& plugin_name : missing_plugins_names)
	{
		report_itr = std::ranges::copy(plugin_name, report_itr).out;
		*report_itr = '\n';
		++report_itr;
	}
	return report;
}

}

namespace josk
{

std::expected<std::vector<task::task_load_file>, std::string> find_plugins(const cli::args& arguments)
{
	auto order_result = parse_load_order(arguments.load_order_path);
	if (!order_result.has_value())
	{
		return std::unexpected(order_result.error());
	}
	load_order_data load_order_data = std::move(order_result.value());
	std::vector<task::task_load_file> result{};
	auto path_skyrim_esm = fs::path{arguments.skyrim_data_path} / "Skyrim.esm";
	check_add_file(std::move(path_skyrim_esm), result, load_order_data);

	for (fs::recursive_directory_iterator itr{arguments.mods_path}; itr != fs::recursive_directory_iterator{}; ++itr)
	{
		if (itr.depth() > 2U)
		{
			itr.disable_recursion_pending();
		}
		if (!fs::is_directory(itr->path()))
		{
			check_add_file(itr->path(), result, load_order_data);
		}
	}

	if (!load_order_data.empty())
	{
		return std::unexpected(report_missing_plugins(load_order_data));
	}
	return result;
}

}
