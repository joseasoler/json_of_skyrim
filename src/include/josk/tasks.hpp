#pragma once

#include <josk/tes_format.hpp>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace josk::task
{

using load_order_t = std::int32_t;
constexpr load_order_t invalid_order{-1};

/** Maps each plugin name to its priority. */
using load_order_data = std::unordered_map<std::string, load_order_t>;

/** Parse the file detailing the load order of all plugins. */
std::expected<load_order_data, std::string> get_load_order(const std::filesystem::path& load_order_path);

struct plugin_file
{
	load_order_t load_order;
	std::filesystem::path path;
};

using plugin_files = std::vector<plugin_file>;

/** List of plugin files to be loaded, sorted by inverse load order. */
std::expected<plugin_files, std::string> find_plugins(
		const std::filesystem::path& skyrim_data_path, load_order_data& load_order, const std::filesystem::path& mods_path
);

/** Maps the formid of each unparsed record to its unparsed data field. */
using record_group = std::unordered_map<tes::formid_t, std::vector<char>>;

/** Maps record type to all records of that type. */
using record_groups = std::unordered_map<tes::record_type, record_group>;

/** Loads plugin files, preparses its records, and merges them with other files. */
std::expected<record_groups, std::string> preparse_and_merge_plugins(const plugin_files& plugins);

}
