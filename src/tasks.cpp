#include <josk/tasks.hpp>
#include <josk/tes_format.hpp>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
namespace fs = std::filesystem;
using namespace josk::task;
using namespace josk::tes;

void check_add_file(const fs::path& path, std::vector<plugin_file_t>& files, load_order_t& load_order_data)
{
	const auto itr = load_order_data.find(path.filename().string());
	if (itr == load_order_data.cend() || !fs::is_regular_file(path))
	{
		return;
	}

	auto& [load_order, path_ref] = files.emplace_back();
	load_order = itr->second;
	path_ref = path;
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

template <std::integral integral_type>
integral_type parse_integral(std::ifstream& input)
{
	integral_type value{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	input.read(reinterpret_cast<char*>(&value), sizeof(value));
	return value;
}

record_type_t parse_record_type(std::ifstream& input)
{
	return static_cast<record_type_t>(parse_integral<std::underlying_type_t<record_type_t>>(input));
}

void seek_ahead(std::ifstream& input, const tes_size_t jump_size)
{
	input.seekg(input.tellg() + static_cast<std::ifstream::pos_type>(jump_size));
}

const std::unordered_set<record_type_t>& requested_records()
{
	static const std::unordered_set records{record_type_t::avif, record_type_t::perk};
	return records;
}

std::expected<void, std::string> preparse_record(std::ifstream& input, record_group_t& group)
{
	// Parse required header values and ignore the rest.
	seek_ahead(input, record_type_size);
	const auto record_data_size = parse_integral<tes_size_t>(input);
	seek_ahead(input, tes_size_of<std::uint32_t>()); // Flags
	const auto record_id = parse_integral<formid_t>(input);

	constexpr tes_size_t header_remaining_size = record_header_size - record_type_size - tes_size_of<tes_size_t>() -
																							 tes_size_of<std::uint32_t>() - tes_size_of<formid_t>();
	if (group.contains(record_id))
	{
		// If this record group already contains the parsed formid, it must come from a file with higher priority.
		// This record can be ignored.
		seek_ahead(input, header_remaining_size + record_data_size);
		return {};
	}
	seek_ahead(input, header_remaining_size);

	// Initialize a new record and place the raw data in it.
	auto& record_data = group[record_id];
	record_data.resize(record_data_size);
	input.read(record_data.data(), static_cast<std::ifstream::pos_type>(record_data_size));
	return {};
}

std::expected<void, std::string> preparse_group(std::ifstream& input, plugin_groups_t& groups)
{
	// input has read the GRUP record type. Skip the GRUP header.
	const auto grup_total_size = parse_integral<tes_size_t>(input);
	constexpr tes_size_t grup_header_remaining_size = group_header_size - record_type_size - tes_size_of<tes_size_t>();
	seek_ahead(input, grup_header_remaining_size);

	// Unlike records, the data field includes header size.
	if (grup_total_size == group_header_size)
	{
		// Header-only group without data. The reader is already pointing to the next group header. Skip and carry on.
		return {};
	}
	const tes_size_t grup_data_size = grup_total_size - group_header_size;
	const tes_size_t grup_remaining_data_size = grup_data_size - record_type_size;
	if (const auto grup_contained_record_type = parse_record_type(input);
			requested_records().contains(grup_contained_record_type))
	{
		// Seek back to the beginning of the first record.
		input.seekg(input.tellg() - static_cast<std::ifstream::pos_type>(record_type_size));

		const auto group_data_end = input.tellg() + static_cast<std::ifstream::pos_type>(grup_data_size);
		record_group_t& group = groups[grup_contained_record_type];
		while (input.tellg() < group_data_end)
		{
			if (auto record_result = preparse_record(input, group); !record_result.has_value())
			{
				return std::unexpected(record_result.error());
			}
		}
		assert(input.tellg() == group_data_end);
	}
	else
	{
		seek_ahead(input, grup_remaining_data_size);
	}

	return {};
}

std::expected<void, std::string> preparse_file(const plugin_file_t& plugin_file, plugin_groups_t& groups)
{
	constexpr auto open_flags = static_cast<std::ios_base::openmode>(
			static_cast<unsigned int>(std::ios::binary) | static_cast<unsigned int>(std::ios::in)
	);

	const auto& file_path = plugin_file.path;
	std::ifstream input(file_path, open_flags);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not open file {}.", file_path.string()));
	}

	// Check and skip TES4 record.
	if (const auto tes4_record_type = parse_record_type(input); tes4_record_type != record_type_t::tes4 || !input.good())
	{
		return std::unexpected(std::format("{} is not a TES4 file", file_path.string()));
	}
	// Remaining header size after parsing record type and data size.
	constexpr tes_size_t header_remaining_size = record_header_size - record_type_size - tes_size_of<tes_size_t>();
	seek_ahead(input, parse_integral<tes_size_t>(input) + header_remaining_size);

	// Iterate over the GRUPs contained in the plugin file.
	auto grup_record_type = parse_record_type(input);
	while (!input.eof())
	{
		if (grup_record_type != record_type_t::grup || !input.good())
		{
			const auto position = static_cast<std::int64_t>(input.tellg());
			return std::unexpected(
					std::format("Error during GRUP processing at position {} of file {}.", position, file_path.string())
			);
		}

		if (auto preparse_group_result = preparse_group(input, groups); !preparse_group_result.has_value())
		{
			return std::unexpected(preparse_group_result.error());
		}

		// Read the grup id of the next iteration.
		grup_record_type = parse_record_type(input);
	}

	return {};
}

}

namespace josk::task
{

std::expected<load_order_t, std::string> parse_load_order(const fs::path& load_order_path)
{
	std::ifstream input(load_order_path, std::ios::in);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not load order file {}.", load_order_path.generic_string()));
	}

	load_order_t data{};
	order_t order{};
	for (std::string line; std::getline(input, line);)
	{
		if (line.front() == '#')
		{
			continue;
		}
		data[std::move(line)] = ++order;
	}
	return data;
}

std::expected<std::vector<plugin_file_t>, std::string> find_plugins(
		const fs::path& skyrim_data_path, load_order_t& load_order, const fs::path& mods_path
)
{
	std::vector<plugin_file_t> files{};
	for (fs::recursive_directory_iterator itr{mods_path}; itr != fs::recursive_directory_iterator{}; ++itr)
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

	const auto path_skyrim_esm = fs::path{skyrim_data_path} / "Skyrim.esm";
	check_add_file(path_skyrim_esm, files, load_order);

	std::ranges::sort(
			files, [](const plugin_file_t& lhs, const plugin_file_t& rhs) { return lhs.load_order > rhs.load_order; }
	);

	if (files.size() < load_order.size())
	{
		return std::unexpected(report_missing_plugins(load_order));
	}

	return files;
}

std::expected<plugin_groups_t, std::string> preparse_and_merge_plugins(const std::vector<plugin_file_t>& plugins)
{
	plugin_groups_t groups{};
	// Files are read in inverse priority order. Preparsing chooses the latest version of each record.
	for (const auto& plugin : plugins)
	{
		if (auto result = preparse_file(plugin, groups); !result.has_value())
		{
			return std::unexpected(result.error());
		}
	}

	return groups;
}

}
