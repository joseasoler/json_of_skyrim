#include <josk/cli.hpp>

#include <CLI/App.hpp>
#include <CLI/Validators.hpp>

namespace josk::cli
{

void configure_cli(CLI::App& app, args& arguments)
{
	app.name("josk");
	app.description("JSON of Skyrim - Generate JSON data from a Skyrim modlist.");

	app.add_option("-o,--order", arguments.load_order_path, "Path to loadorder.txt.")
			->required(true)
			->check(CLI::ExistingFile);
	app.add_option("-d,--data", arguments.skyrim_data_path, "Path to Skyrim\\Data folder.")
			->required(true)
			->check(CLI::ExistingDirectory);
	app.add_option("-m,--mods", arguments.mods_path, "Path to mods folder.")
			->required(true)
			->check(CLI::ExistingDirectory);
}

}
