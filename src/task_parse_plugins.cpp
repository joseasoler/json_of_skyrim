#include <josk/tasks.hpp>
#include <josk/tes_format.hpp>
#include <josk/tes_parse.hpp>

#include <cassert>
#include <cstdint>
#include <expected>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

namespace
{
using namespace josk::task;
using namespace josk::tes;

std::expected<void, std::string> parse_record(std::ifstream& input, record_group_t& group)
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

std::expected<void, std::string> parse_group(std::ifstream& input, plugin_groups_t& groups)
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
			required_records().contains(grup_contained_record_type))
	{
		// Seek back to the beginning of the first record.
		input.seekg(input.tellg() - static_cast<std::ifstream::pos_type>(record_type_size));

		const auto group_data_end = input.tellg() + static_cast<std::ifstream::pos_type>(grup_data_size);
		record_group_t& group = groups[grup_contained_record_type];
		while (input.tellg() < group_data_end)
		{
			if (auto record_result = parse_record(input, group); !record_result.has_value())
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

std::expected<void, std::string> parse_plugin(const plugin_t& plugin, plugin_groups_t& groups)
{
	constexpr auto open_flags = static_cast<std::ios_base::openmode>(
			static_cast<unsigned int>(std::ios::binary) | static_cast<unsigned int>(std::ios::in)
	);

	std::ifstream input(plugin.path, open_flags);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not open file {}.", plugin.filename));
	}

	// Check and skip TES4 record.
	if (const auto tes4_result = parse_tes4_record(input); !tes4_result.has_value())
	{
		return std::unexpected(std::format("{}: {}", plugin.filename, tes4_result.error()));
	}

	// Iterate over the GRUPs contained in the plugin file.
	auto grup_record_type = parse_record_type(input);
	while (!input.eof())
	{
		if (grup_record_type != record_type_t::grup || !input.good())
		{
			const auto position = static_cast<std::int64_t>(input.tellg());
			return std::unexpected(
					std::format("Error during GRUP processing at position {} of file {}.", position, plugin.filename)
			);
		}

		if (auto parse_group_result = parse_group(input, groups); !parse_group_result.has_value())
		{
			return std::unexpected(parse_group_result.error());
		}

		// Read the grup id of the next iteration.
		grup_record_type = parse_record_type(input);
	}

	return {};
}

}

namespace josk::task
{

std::expected<plugin_groups_t, std::string> parse_plugins(const std::vector<plugin_t>& plugins)
{
	plugin_groups_t groups{};

	// Files are read in inverse priority order. When an existing formid is found on a later file, it can be ignored.
	for (const auto& plugin : plugins | std::views::reverse)
	{
		if (auto result = parse_plugin(plugin, groups); !result.has_value())
		{
			return std::unexpected(result.error());
		}
	}

	return groups;
}

}
