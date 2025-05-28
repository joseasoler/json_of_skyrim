#include <josk/tasks.hpp>

#include <expected>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <utility>

namespace josk::task
{

std::expected<parse_data_t, std::string> parse_load_order(parse_data_t parse_data)
{
	auto& load_order_path = parse_data.arguments.load_order_path;
	std::ifstream input(load_order_path, std::ios::in);
	load_order_path.clear();

	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not load order file {}.", load_order_path.generic_string()));
	}

	auto& load_order = parse_data.load_order;
	order_t order{};
	for (std::string line; std::getline(input, line);)
	{
		if (line.front() == '#')
		{
			continue;
		}
		load_order[std::move(line)] = ++order;
	}

	return parse_data;
}

}
