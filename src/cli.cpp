#include <josk/cli.hpp>

#include <CLI/App.hpp>

#include <expected>
#include <filesystem>
#include <format>
#include <string>

namespace josk::cli
{

void configure_cli(CLI::App& app, arguments_t& arguments)
{
	app.name("josk");
	app.description("Generate JSON data from a Skyrim modlist.");

	app.add_option("-p,--profile", arguments.profile_path, "Path to Mod Organizer 2 profile..")->required(true);
	app.add_option("-d,--data", arguments.data_path, "Path to Data folder.")->required(true);
	app.add_option("-m,--mods", arguments.mods_path, "Path to mods folder.")->required(true);
	app.add_option("-o,--output", arguments.output_path, "Path to output folder.")->required(true);
}

std::expected<arguments_t, std::string> validate_arguments(arguments_t arguments)
{
	namespace fs = std::filesystem;
	if (!fs::exists(arguments.profile_path))
	{
		return std::unexpected(std::format("Profile path {} does not exist.", arguments.profile_path.string()));
	}
	if (!fs::is_directory(arguments.profile_path))
	{
		return std::unexpected(std::format("Profile path {} is not a directory.", arguments.profile_path.string()));
	}

	if (!fs::exists(arguments.data_path))
	{
		return std::unexpected(std::format("Data path {} does not exist.", arguments.data_path.string()));
	}
	if (!fs::is_directory(arguments.data_path))
	{
		return std::unexpected(std::format("Data path {} is not a directory.", arguments.data_path.string()));
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

	return arguments;
}

}
