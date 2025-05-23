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
struct args final
{
	std::filesystem::path load_order_path;
	std::filesystem::path skyrim_data_path;
	std::filesystem::path mods_path;
	std::filesystem::path output_path;
};

void configure_cli(CLI::App& app, args& arguments);

std::expected<void, std::string> validate_arguments(const args& arguments);

}
