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

using load_order_t = std::unordered_map<std::string, order_t>;

struct parse_data_t final
{
	cli::arguments_t arguments;
	/** Maps each plugin name to its priority. */
	load_order_t load_order;
	/** List of plugin files to be loaded, sorted by inverse load order. */
	std::vector<std::filesystem::path> plugins;
};

std::expected<parse_data_t, std::string> initialize_parsing(cli::arguments_t arguments);

/** Parse the file detailing the load order of all plugins. */
std::expected<parse_data_t, std::string> parse_load_order(parse_data_t parse_data);

/** List of plugin files to be loaded, sorted by inverse load order. */
std::expected<parse_data_t, std::string> find_plugins(parse_data_t parse_data);

/** Maps the formid of each unparsed record to its unparsed data field. */
using record_group_t = std::unordered_map<tes::formid_t, std::vector<char>>;

/** Maps record type to all records of that type. */
using plugin_groups_t = std::unordered_map<tes::record_type_t, record_group_t>;

/** Loads plugin files, parses its records, and merges them with other files. */
std::expected<plugin_groups_t, std::string> parse_plugins(const parse_data_t& parse_data);

}
