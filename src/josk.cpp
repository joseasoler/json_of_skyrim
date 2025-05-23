#include <josk/cli.hpp>
#include <josk/tes_format.hpp>
#include <josk/tes_reader.hpp>

#include <CLI/App.hpp>

#include <cstdio>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <print>
#include <string>

int main(const int argc, char* argv[])
{
	CLI::App app{};
	josk::cli::args arguments{};
	josk::cli::configure_cli(app, arguments);
	CLI11_PARSE(app, argc, argv);
	if (const auto validation = josk::cli::validate_arguments(arguments); !validation.has_value())
	{
		std::println(stderr, "{}", validation.error());
		return EXIT_FAILURE;
	}
	// ToDo carry on with using the arguments properly and implementing the rest of the data flow.

	const auto skyrim_esm_path = arguments.skyrim_data_path / "Skyrim.esm";
	const auto result = josk::tes::read_file(skyrim_esm_path.string().data(), {josk::tes::record_type::avif});

	return EXIT_SUCCESS;
}
