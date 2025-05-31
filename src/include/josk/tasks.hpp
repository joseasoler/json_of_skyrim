#pragma once

#include <josk/cli.hpp>
#include <josk/tes_parse.hpp>

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
	std::filesystem::path data_path;
	std::filesystem::path mods_path;
	/** Maps each plugin name to its priority. */
	std::unordered_map<std::string, order_t> load_order;
};

/** Parse the file detailing plugin load order. */
std::expected<plugins_to_load_t, std::string> parse_load_order(cli::arguments_t arguments);

/** List of plugin files to be loaded, sorted by inverse load order. */
std::expected<std::vector<plugin_t>, std::string> find_plugins(plugins_to_load_t modlist);

/** Loads plugin files and parses the final version of each record. */
std::expected<tes::parsed_records_t, std::string> parse_plugins(const std::vector<plugin_t>& plugins);

}
