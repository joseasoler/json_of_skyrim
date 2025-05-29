#include <josk/cli.hpp>
#include <josk/tasks.hpp>

#include <expected>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <utility>

namespace josk::task
{

std::expected<plugins_to_load_t, std::string> parse_load_order(cli::arguments_t arguments)
{
	const auto& load_order_path = arguments.load_order_path;
	std::ifstream input(load_order_path, std::ios::in);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not load order file {}.", load_order_path.generic_string()));
	}

	plugins_to_load_t plugins_to_load;
	plugins_to_load.skyrim_data_path = std::move(arguments.skyrim_data_path);
	plugins_to_load.mods_path = std::move(arguments.mods_path);

	auto& load_order = plugins_to_load.load_order;
	order_t order{};
	for (std::string line; std::getline(input, line);)
	{
		if (line.front() == '#')
		{
			continue;
		}
		load_order[std::move(line)] = ++order;
	}

	return plugins_to_load;
}

}
