#include <josk/tasks.hpp>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{

namespace fs = std::filesystem;
using namespace josk::task;

void try_add_plugin(
		const fs::path& filesystem_path, std::vector<plugin_t>& files, std::unordered_map<std::string, order_t>& load_order
)
{
	if (!fs::is_regular_file(filesystem_path))
	{
		// Modlist TES files are expected to be regular files.
		return;
	}

	auto filename_to_test = filesystem_path.filename().string();
	const auto itr = load_order.find(filename_to_test);
	if (itr == load_order.cend())
	{
		// The plugin filename must be contained in the list of remaining plugins to find.
		return;
	}

	auto& [plugin_order, plugin_filename, plugin_path] = files.emplace_back();
	plugin_order = itr->second;
	plugin_filename = std::move(filename_to_test);
	plugin_path = filesystem_path;
	// Remove the found plugin from the remaining load order.
	load_order.erase(itr);
}

std::string report_missing_plugins(const std::unordered_map<std::string, order_t>& load_order)
{
	std::size_t missing_plugins_text_size{};
	std::vector<std::string_view> missing_plugins_names{};
	for (const auto& plugin_name : load_order | std::views::keys)
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

std::expected<std::vector<plugin_t>, std::string> find_plugins(plugins_to_load_t modlist)
{
	std::vector<plugin_t> files;
	auto& load_order = modlist.load_order;
	for (fs::recursive_directory_iterator itr{modlist.mods_path}; itr != fs::recursive_directory_iterator{}; ++itr)
	{
		if (itr.depth() > 2U)
		{
			itr.disable_recursion_pending();
		}
		if (!fs::is_directory(itr->path()))
		{
			try_add_plugin(itr->path(), files, load_order);
		}
	}

	// ToDo check all files in this folder.
	const auto path_skyrim_esm = fs::path{modlist.skyrim_data_path} / "Skyrim.esm";
	try_add_plugin(path_skyrim_esm, files, load_order);

	if (!load_order.empty())
	{
		return std::unexpected(report_missing_plugins(load_order));
	}

	std::ranges::sort(files);

	return files;
}

}
