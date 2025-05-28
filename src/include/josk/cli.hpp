#pragma once

#include <expected>
#include <filesystem>
#include <string>

namespace CLI
{
class App;
}

namespace josk::cli
{
struct arguments_t final
{
	std::filesystem::path load_order_path;
	std::filesystem::path skyrim_data_path;
	std::filesystem::path mods_path;
	std::filesystem::path output_path;
};

void configure_cli(CLI::App& app, arguments_t& arguments);

std::expected<arguments_t, std::string> validate_arguments(arguments_t arguments);

}
