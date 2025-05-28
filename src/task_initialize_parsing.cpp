#include <josk/cli.hpp>
#include <josk/tasks.hpp>

#include <expected>
#include <string>
#include <utility>

namespace josk::task
{

std::expected<parse_data_t, std::string> initialize_parsing(cli::arguments_t arguments)
{
	parse_data_t parse_data;
	parse_data.arguments = std::move(arguments);
	return parse_data;
}

}
