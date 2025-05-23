#include <josk/cli.hpp>

#include <CLI/App.hpp>

#include <expected>
#include <filesystem>
#include <format>
#include <string>

namespace josk::cli
{

void configure_cli(CLI::App& app, args& arguments)
{
	app.name("josk");
	app.description("JSON of Skyrim - Generate JSON data from a Skyrim modlist.");

	app.add_option("-l,--loadorder", arguments.load_order_path, "Path to loadorder.txt.")->required(true);
	app.add_option("-d,--data", arguments.skyrim_data_path, "Path to Skyrim\\Data folder.")->required(true);
	app.add_option("-m,--mods", arguments.mods_path, "Path to mods folder.")->required(true);
	app.add_option("-o,--output", arguments.output_path, "Path to output folder.")->required(true);
}

std::expected<void, std::string> validate_arguments(const args& arguments)
{
	namespace fs = std::filesystem;
	if (!fs::exists(arguments.load_order_path))
	{
		return std::unexpected(std::format("Load order file {} does not exist.", arguments.load_order_path.string()));
	}
	if (!fs::is_regular_file(arguments.load_order_path))
	{
		return std::unexpected(
				std::format("Load order path {} is not a regular file.", arguments.load_order_path.string())
		);
	}

	if (!fs::exists(arguments.skyrim_data_path))
	{
		return std::unexpected(std::format("Skyrim data path {} does not exist.", arguments.skyrim_data_path.string()));
	}
	if (!fs::is_directory(arguments.skyrim_data_path))
	{
		return std::unexpected(std::format("Skyrim path {} is not a directory.", arguments.skyrim_data_path.string()));
	}

	if (!fs::exists(arguments.mods_path))
	{
		return std::unexpected(std::format("Mods path {} does not exist.", arguments.mods_path.string()));
	}
	if (!fs::is_directory(arguments.mods_path))
	{
		return std::unexpected(std::format("Mods {} is not a directory.", arguments.mods_path.string()));
	}

	if (!fs::exists(arguments.output_path))
	{
		return std::unexpected(std::format("Output path {} does not exist.", arguments.output_path.string()));
	}
	if (!fs::is_directory(arguments.output_path))
	{
		return std::unexpected(std::format("Output {} is not a directory.", arguments.output_path.string()));
	}

	return {};
}

}
