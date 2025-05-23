#include <josk/cli.hpp>
#include <josk/tes_format.hpp>
#include <josk/tes_reader.hpp>

#include <CLI/App.hpp>

#include <filesystem>

int main(const int argc, char* argv[])
{
	CLI::App app{};
	josk::cli::args arguments{};
	josk::cli::configure_cli(app, arguments);
	CLI11_PARSE(app, argc, argv);

	// ToDo carry on with using the arguments properly and implementing the rest of the data flow.

	const auto skyrim_esm_path = std::filesystem::path{arguments.skyrim_data_path} / "Skyrim.esm";
	const auto result = josk::tes::read_file(skyrim_esm_path.string().data(), {josk::tes::record_type::avif});

	return static_cast<int>(!result.has_value());
}
