#include <josk/cli.hpp>
#include <josk/tasks.hpp>

#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <utility>

namespace josk::task
{

std::expected<plugins_to_load_t, std::string> parse_load_order(cli::arguments_t arguments)
{
	const auto load_order_path = arguments.profile_path / "loadorder.txt";
	if (!std::filesystem::is_regular_file(load_order_path))
	{
		return std::unexpected(std::format("Could not find load order file {}.", load_order_path.generic_string()));
	}
	std::ifstream input(load_order_path, std::ios::in);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not open load order file {}.", load_order_path.generic_string()));
	}

	plugins_to_load_t plugins_to_load;
	plugins_to_load.data_path = std::move(arguments.data_path);
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
