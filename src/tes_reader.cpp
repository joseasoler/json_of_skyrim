#include <josk/tes_format.hpp>
#include <josk/tes_reader.hpp>

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <unordered_set>
#include <utility>

namespace
{

using josk::tes::record_type;

template <std::integral integral_type>
integral_type read_integral(std::ifstream& input)
{
	integral_type value{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	input.read(reinterpret_cast<char*>(&value), sizeof(value));
	return value;
}

record_type read_record_type(std::ifstream& input)
{
	return static_cast<record_type>(read_integral<std::uint32_t>(input));
}

void jump_ahead(std::ifstream& input, const std::uint32_t jump_size)
{
	input.seekg(input.tellg() + static_cast<std::ifstream::pos_type>(jump_size));
}

struct reader_data final
{
	const char* path{};
	const std::unordered_set<record_type>* requested_record_types;
	std::ifstream input;
	josk::tes::raw_record_groups records;
};

std::expected<reader_data, std::string> initialize_reader(
		const char* path, const std::unordered_set<record_type>& requested_record_types
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

	reader_data data{};
	data.path = path;
	data.requested_record_types = &requested_record_types;
	return data;
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
	constexpr std::uint32_t record_header_remaining_size = (sizeof(std::uint32_t) * 2U) + (sizeof(std::uint16_t) * 4U);
	const auto tes4_record_remaining_size = read_integral<josk::tes::record_size_t>(input) + record_header_remaining_size;
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
			return std::unexpected("Error during GRUP processing.");
		}

		constexpr auto grup_remaining_header_size =
				(sizeof(std::uint8_t) * 4U) + sizeof(std::int32_t) + (sizeof(std::uint16_t) * 2U) + sizeof(std::uint32_t);
		constexpr auto grup_total_header_size = grup_remaining_header_size + 8U;

		const std::uint32_t grup_remaining_data_size =
				read_integral<std::uint32_t>(input) - grup_total_header_size - sizeof(record_type);
		jump_ahead(input, grup_remaining_header_size);

		if (const auto grup_contained_record_type = read_record_type(input);
				data.requested_record_types->contains(grup_contained_record_type))
		{
			assert(!data.records.contains(grup_contained_record_type));
			auto& record_group_data = data.records[grup_contained_record_type];
			record_group_data.resize(grup_remaining_data_size);
			input.read(record_group_data.data(), grup_remaining_data_size);
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

}

namespace josk::tes
{

std::expected<raw_record_groups, std::string> read_file(
		const char* path, const std::unordered_set<record_type>& requested_record_types
)
{
	auto data = initialize_reader(path, requested_record_types)
									.and_then(open_file)
									.and_then(validate_tes4_record)
									.and_then(process_grup_records);
	if (data.has_value())
	{
		return std::move(data->records);
	}

	return {};
}

}
