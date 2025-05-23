#include <josk/task_load_order.hpp>
#include <josk/tasks.hpp>

#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <utility>

namespace josk
{

std::expected<task::load_order_data, std::string> parse_load_order(const std::filesystem::path& load_order_path)
{
	std::ifstream input(load_order_path, std::ios::in);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not load order file {}.", load_order_path.generic_string()));
	}

	task::load_order_data data{};
	task::priority_t priority{};
	for (std::string line; std::getline(input, line);)
	{
		if (line.front() == '#')
		{
			continue;
		}
		data[std::move(line)] = ++priority;
	}
	return data;
}

}
