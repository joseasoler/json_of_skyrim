#pragma once

#include <josk/cli.hpp>
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

struct plugin_t final
{
	order_t order;
	std::string filename;
	std::filesystem::path path;
	auto operator<=>(const plugin_t&) const = default;
};

struct plugins_to_load_t final
{
	std::filesystem::path skyrim_data_path;
	std::filesystem::path mods_path;
	/** Maps each plugin name to its priority. */
	std::unordered_map<std::string, order_t> load_order;
};

/** Parse the file detailing plugin load order. */
std::expected<plugins_to_load_t, std::string> parse_load_order(cli::arguments_t arguments);

/** List of plugin files to be loaded, sorted by inverse load order. */
std::expected<std::vector<plugin_t>, std::string> find_plugins(plugins_to_load_t modlist);

/** Maps the formid of each unparsed record to its unparsed data field. */
using record_group_t = std::unordered_map<tes::formid_t, std::vector<char>>;

/** Maps record type to all records of that type. */
using plugin_groups_t = std::unordered_map<tes::record_type_t, record_group_t>;

/** Loads plugin files, parses its records, and merges them with other files. */
std::expected<plugin_groups_t, std::string> parse_plugins(const std::vector<plugin_t>& plugins);

}
