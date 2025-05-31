#include <josk/tasks.hpp>
#include <josk/tes_parse.hpp>

#include <cstdint>
#include <expected>
#include <format>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace
{

[[nodiscard]] std::string_view input_state_string(std::ifstream& input) noexcept
{
	if (input.eof())
	{
		return "eof";
	}

	if (input.fail())
	{
		return "fail";
	}

	if (input.bad())
	{
		return "bad";
	}
	return "good";
}

std::string format_error(const std::string& filename, const std::string& parse_error, std::ifstream& input)
{
	constexpr auto* format_str = "Error while parsing {}: {}. Input state: {}. Input position: {:x}.";
	return std::format(
			format_str, filename, parse_error, input_state_string(input), static_cast<std::uint64_t>(input.tellg())
	);
}

}

namespace josk::task
{

std::expected<tes::parsed_records_t, std::string> parse_plugins(const std::vector<plugin_t>& plugins)
{
	tes::parsed_records_t parsed_records{};
	// Files are read in inverse priority order. The first record we find with a specific formid is always the one that
	// must be kept.
	for (const auto& plugin : plugins | std::views::reverse)
	{
		auto open_plugin_result = tes::open_tes_plugin(plugin.path);
		if (!open_plugin_result.has_value())
		{
			constexpr auto* format_str = "Error while opening {}: {}.";
			return std::unexpected(std::format(format_str, plugin.filename, open_plugin_result.error()));
		}

		auto& input = open_plugin_result.value();
		while (input.good() && !input.eof())
		{
			const auto open_group_result = tes::open_next_group(input);
			if (!open_group_result.has_value())
			{
				return std::unexpected(format_error(plugin.filename, open_group_result.error(), input));
			}

			const auto& group_data = open_group_result.value();
			if (group_data == tes::no_next_grup)
			{
				break;
			}
			if (const auto parse_group_result = tes::parse_group(input, group_data, parsed_records);
					!parse_group_result.has_value())
			{
				return std::unexpected(format_error(plugin.filename, parse_group_result.error(), input));
			}
		}

		if (const auto close_plugin_result = tes::close_tes_plugin(input); !close_plugin_result.has_value())
		{
			return std::unexpected(format_error(plugin.filename, close_plugin_result.error(), input));
		}
	}

	return parsed_records;
}

}
