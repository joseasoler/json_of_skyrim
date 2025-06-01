#include <josk/tasks.hpp>
#include <josk/tes_parse.hpp>

#include <expected>
#include <ranges>
#include <string>
#include <vector>

namespace josk::task
{

std::expected<tes::parsed_records_t, std::string> parse_plugins(const std::vector<plugin_t>& plugins)
{
	tes::parsed_records_t parsed_records{};
	// Files are read in inverse priority order. The first record we find with a specific formid is always the one that
	// must be kept.
	for (const auto& plugin : plugins | std::views::reverse)
	{
		const auto plugin_result = tes::open_plugin(plugin.path, plugin.filename, parsed_records)
																	 .and_then(tes::parse_plugin)
																	 .and_then(tes::close_plugin);
		if (!plugin_result.has_value())
		{
			return std::unexpected(plugin_result.error());
		}
	}

	return parsed_records;
}

}
