#include <josk/tes_format.hpp>
#include <josk/tes_reader.hpp>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace
{

template <std::integral integral_type>
integral_type read_integral(std::ifstream& input)
{
	integral_type value{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	input.read(reinterpret_cast<char*>(&value), sizeof(value));
	return value;
}

using josk::tes::record_type;
record_type read_record_type(std::ifstream& input)
{
	return static_cast<record_type>(read_integral<std::underlying_type_t<record_type>>(input));
}

using josk::tes::tes_size_t;
void jump_ahead(std::ifstream& input, const tes_size_t jump_size)
{
	input.seekg(input.tellg() + static_cast<std::ifstream::pos_type>(jump_size));
}

struct reader_data final
{
	const char* path;
	const std::unordered_set<record_type>& requested_record_types;
	std::ifstream& input;
	josk::tes::raw_record_groups records;

	reader_data(const char* file_path, const std::unordered_set<record_type>& record_types, std::ifstream& input_stream)
		: path{file_path}
		, requested_record_types{record_types}
		, input{input_stream}
	{
	}
};

std::expected<reader_data, std::string> initialize_reader(
		const char* path, const std::unordered_set<record_type>& requested_record_types, std::ifstream& input
)
{
	if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
	{
		return std::unexpected(std::format("Could not find file {}.", path));
	}

	if (requested_record_types.empty())
	{
		return std::unexpected("At least one record type must be requested.");
	}

	return reader_data(path, requested_record_types, input);
}

std::expected<reader_data, std::string> open_file(reader_data data)
{
	constexpr auto open_flags = static_cast<std::ios_base::openmode>(
			static_cast<unsigned int>(std::ios::binary) | static_cast<unsigned int>(std::ios::out)
	);

	data.input.open(data.path, open_flags);
	if (!data.input.is_open() || !data.input.good())
	{
		return std::unexpected(std::format("Could not open file {}.", data.path));
	}

	return data;
}

std::expected<reader_data, std::string> validate_tes4_record(reader_data data)
{
	auto& input = data.input;
	if (const auto tes4_record_type = read_record_type(input); tes4_record_type != record_type::tes4 || !input.good())
	{
		return std::unexpected(std::format("{} is not a TES4 file", data.path));
	}

	// Record header size excluding the record type and data size fields.
	constexpr tes_size_t record_header_remaining_size =
			josk::tes::record_header_size - sizeof(record_type) - sizeof(tes_size_t);

	const tes_size_t tes4_record_remaining_size = read_integral<tes_size_t>(input) + record_header_remaining_size;
	jump_ahead(input, tes4_record_remaining_size);

	return data;
}

std::expected<reader_data, std::string> process_grup_records(reader_data data)
{
	auto& input = data.input;

	auto grup_record_type = read_record_type(input);
	while (!input.eof())
	{
		if (grup_record_type != record_type::grup || !input.good())
		{
			const auto position = static_cast<std::int64_t>(input.tellg());
			return std::unexpected(
					std::format("Error during GRUP processing at position {} of file {}.", position, data.path)
			);
		}

		constexpr tes_size_t grup_header_remaining_size =
				josk::tes::group_header_size - sizeof(record_type) - sizeof(tes_size_t);

		// The data field of GRUP records includes the header size, and the first record type will be read to identify the
		// record type stored in the group.
		const tes_size_t grup_remaining_data_size =
				read_integral<tes_size_t>(input) - josk::tes::group_header_size - sizeof(record_type);

		jump_ahead(input, grup_header_remaining_size);

		if (const auto grup_contained_record_type = read_record_type(input);
				data.requested_record_types.contains(grup_contained_record_type))
		{
			// josk assumes that a single file never has more than one group of the same record type.
			assert(!data.records.contains(grup_contained_record_type));
			auto& record_group_data = data.records[grup_contained_record_type];
			record_group_data.resize(grup_remaining_data_size + sizeof(record_type));
			// Manually copy the first record type to the start of the data.
			std::memcpy(record_group_data.data(), &grup_contained_record_type, sizeof(record_type));
			// Then read the remaining data.
			input.read(record_group_data.data() + sizeof(record_type), grup_remaining_data_size);
		}
		else
		{
			jump_ahead(input, grup_remaining_data_size);
		}

		// Read the record type of the group to be processed in the next iteration.
		grup_record_type = read_record_type(input);
	}

	return data;
}

std::expected<josk::tes::raw_record_groups, std::string> finish_reading(reader_data data)
{
	if (const auto& input = data.input; !input.eof())
	{
		return std::unexpected(std::format("Attempting to close unfinished file {}.", data.path));
	}

	return std::move(data.records);
}

}

namespace josk::tes
{

std::expected<raw_record_groups, std::string> read_file(
		const char* path, const std::unordered_set<record_type>& requested_record_types
)
{
	std::ifstream input;
	auto data = initialize_reader(path, requested_record_types, input)
									.and_then(open_file)
									.and_then(validate_tes4_record)
									.and_then(process_grup_records)
									.and_then(finish_reading);

	input.close();

	return data;
}

}
