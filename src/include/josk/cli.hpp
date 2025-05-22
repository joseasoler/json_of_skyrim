#pragma once

#include <string>

namespace CLI
{
class App;
}

namespace josk::cli
{
struct args final
{
	std::string load_order_path;
	std::string skyrim_data_path;
	std::string mods_path;
};

void configure_cli(CLI::App& app, args& arguments);

}
