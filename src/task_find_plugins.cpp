#include <josk/tasks.hpp>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{

namespace fs = std::filesystem;
using namespace josk::task;

struct plugin_file_t
{
	order_t load_order{};
	std::filesystem::path path;
	auto operator<=>(const plugin_file_t&) const = default;
};

void check_add_file(fs::path path, std::vector<plugin_file_t>& files, load_order_t& load_order_data)
{
	const auto itr = load_order_data.find(path.filename().string());
	if (itr == load_order_data.cend() || !fs::is_regular_file(path))
	{
		return;
	}

	auto& [load_order, path_ref] = files.emplace_back();
	load_order = itr->second;
	path_ref = std::move(path);
	load_order_data.erase(itr);
}

std::string report_missing_plugins(const load_order_t& load_order_data)
{
	std::size_t missing_plugins_text_size{};
	std::vector<std::string_view> missing_plugins_names{};
	for (const auto& plugin_name : load_order_data | std::views::keys)
	{
		missing_plugins_names.emplace_back(plugin_name);
		// Each plugin will have an EOL character.
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

namespace josk::task
{

std::expected<parse_data_t, std::string> find_plugins(parse_data_t parse_data)
{
	std::vector<plugin_file_t> files;
	auto& load_order = parse_data.load_order;
	for (fs::recursive_directory_iterator itr{parse_data.arguments.mods_path}; itr != fs::recursive_directory_iterator{};
			 ++itr)
	{
		if (itr.depth() > 2U)
		{
			itr.disable_recursion_pending();
		}
		if (!fs::is_directory(itr->path()))
		{
			check_add_file(itr->path(), files, load_order);
		}
	}
	parse_data.arguments.mods_path.clear();

	// ToDo check all files in this folder.
	const auto path_skyrim_esm = fs::path{parse_data.arguments.skyrim_data_path} / "Skyrim.esm";
	check_add_file(path_skyrim_esm, files, load_order);
	parse_data.arguments.skyrim_data_path.clear();

	if (files.size() < load_order.size())
	{
		return std::unexpected(report_missing_plugins(load_order));
	}

	std::ranges::sort(files);
	for (auto& [_, path] : files)
	{
		parse_data.plugins.emplace_back(std::move(path));
	}

	return parse_data;
}

}
