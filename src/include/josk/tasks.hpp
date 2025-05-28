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

using order_t = std::int32_t;
constexpr order_t invalid_order{-1};

/** Maps each plugin name to its priority. */
using load_order_t = std::unordered_map<std::string, order_t>;

/** Parse the file detailing the load order of all plugins. */
std::expected<load_order_t, std::string> parse_load_order(const std::filesystem::path& load_order_path);

struct plugin_file_t
{
	order_t load_order;
	std::filesystem::path path;
};

/** List of plugin files to be loaded, sorted by inverse load order. */
std::expected<std::vector<plugin_file_t>, std::string> find_plugins(
		const std::filesystem::path& skyrim_data_path, load_order_t& load_order, const std::filesystem::path& mods_path
);

/** Maps the formid of each unparsed record to its unparsed data field. */
using record_group_t = std::unordered_map<tes::formid_t, std::vector<char>>;

/** Maps record type to all records of that type. */
using plugin_groups_t = std::unordered_map<tes::record_type_t, record_group_t>;

/** Loads plugin files, preparses its records, and merges them with other files. */
std::expected<plugin_groups_t, std::string> preparse_and_merge_plugins(const std::vector<plugin_file_t>& plugins);

}
