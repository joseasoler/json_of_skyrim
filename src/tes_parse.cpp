#include <josk/tes_format.hpp>
#include <josk/tes_parse.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace
{

using namespace josk::tes;

void seek_ahead(std::ifstream& input, const record_size_t seek_distance)
{
	input.seekg(input.tellg() + static_cast<std::ifstream::pos_type>(seek_distance));
}

/**
 * Size of a type in bytes in a TES file.
 * @tparam type Type to check.
 * @return Size in bytes.
 */
template <typename type>
consteval record_size_t tes_size_of()
{
	return static_cast<record_size_t>(sizeof(type));
}

/** In TES files, records are represented as a char[4]. */
constexpr record_size_t record_header_size = record_type_size + tes_size_of<record_size_t>() +
																						 tes_size_of<std::uint32_t>() + tes_size_of<formid_t>() +
																						 (tes_size_of<std::uint16_t>() * 4U);

using field_type_t = std::array<char, record_type_size>;
using field_size_t = std::uint16_t;
consteval field_type_t to_field_type_id(std::string_view field_type_str)
{
	field_type_t field_type{};
	if (field_type_str.size() != field_type.size())
	{
		throw std::invalid_argument("Field type strings must have a size of 4.");
	}

	std::ranges::copy(field_type_str, field_type.begin());
	return field_type;
}

namespace field_type
{
constexpr field_type_t desc = to_field_type_id("DESC");
constexpr field_type_t edid = to_field_type_id("EDID");
constexpr field_type_t full = to_field_type_id("FULL");
}

template <std::integral integral_type>
integral_type parse_integral(std::ifstream& input)
{
	integral_type value{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	input.read(reinterpret_cast<char*>(&value), sizeof(value));
	return input.good() ? value : std::numeric_limits<integral_type>::min();
}

record_type_t parse_record_type(std::ifstream& input)
{
	std::array<char, record_type_size> record_string{};
	input.read(record_string.data(), record_string.size());
	return input.good() ? to_record_type(std::string_view(record_string.data(), record_string.size()))
											: record_type_t::none;
}

bool validate_field_type(std::ifstream& input, const field_type_t& field_type)
{
	field_type_t field_data{};
	input.read(field_data.data(), field_data.size());
	return std::ranges::equal(field_type, field_data);
}

std::string parse_string_field(std::ifstream& input, const field_size_t field_data_size)
{
	std::string result(field_data_size, '\0');
	input.read(result.data(), field_data_size);
	return result;
}

std::expected<void, std::string> validate_and_skip_tes4_record(std::ifstream& input)
{
	if (const auto tes4_record_type = parse_record_type(input); tes4_record_type != record_type_t::tes4)
	{
		return std::unexpected("Could not parse TES4 record");
	}

	// Remaining header size after parsing record type and data size.
	constexpr record_size_t header_remaining_size = record_header_size - record_type_size - tes_size_of<record_size_t>();
	seek_ahead(input, parse_integral<record_size_t>(input) + header_remaining_size);
	return {};
}

struct record_header_data final
{
	record_size_t data_size;
	formid_t record_id;
};

std::expected<record_header_data, std::string> parse_record_header(std::ifstream& input)
{
	// Record type is known.
	seek_ahead(input, record_type_size);
	record_header_data header_data{};
	header_data.data_size = parse_integral<record_size_t>(input);
	// Flags are currently not required by josk.
	seek_ahead(input, tes_size_of<std::uint32_t>());
	header_data.record_id = parse_integral<formid_t>(input);
	constexpr record_size_t header_remaining_size = record_header_size - record_type_size - tes_size_of<record_size_t>() -
																									tes_size_of<std::uint32_t>() - tes_size_of<formid_t>();
	seek_ahead(input, header_remaining_size);
	if (!input.good())
	{
		return std::unexpected("Could not parse record header due to invalid input stream state");
	}

	return header_data;
}

bool is_already_parsed(
		const record_type_t record_type, const formid_t record_id, const parsed_records_t& parsed_records
)
{
	switch (record_type)
	{
		case record_type_t::avif:
			return parsed_records.avif_records.contains(record_id);
		default:
			break;
	}

	assert(false);
	return false;
}

std::expected<void, std::string> parse_avif_data(
		std::ifstream& input, const record_header_data header_data, parsed_records_t& parsed_records
)
{
	seek_ahead(input, header_data.data_size);
	parsed_records.avif_records[header_data.record_id] = {};
	return {};
}

/**
 * These functions expect the stream to be at the start of the data field of the record.
 * Checking stream state before and after the call is the responsibility of the caller.
 * The caller must also set the stream to the end of the record data position after the call.
 */
using parse_record_data_func =
		std::expected<void, std::string> (*)(std::ifstream&, record_header_data, parsed_records_t&);

parse_record_data_func get_parse_record_data_function(const record_type_t record_type)
{
	switch (record_type)
	{
		case record_type_t::avif:
			return parse_avif_data;
		default:
			break;
	}
	return nullptr;
}

}

namespace josk::tes
{

std::expected<std::ifstream, std::string> open_tes_plugin(const std::filesystem::path& path)
{
	constexpr auto open_flags = static_cast<std::ios_base::openmode>(
			static_cast<unsigned int>(std::ios::binary) | static_cast<unsigned int>(std::ios::in)
	);
	std::ifstream input(path, open_flags);
	if (!input.good())
	{
		return std::unexpected("Could not open file");
	}

	if (const auto tes4_result = validate_and_skip_tes4_record(input); !tes4_result.has_value())
	{
		return std::unexpected(tes4_result.error());
	}

	return input;
}

std::expected<grup_data_t, std::string> open_next_group(std::ifstream& input)
{
	if (!input.good())
	{
		// open_next_group is used to iterate over the file, and it is not responsible for EOF checks.
		// This explicit input state check reports iteration errors made by the caller more clearly than a NONE record type.
		return std::unexpected("Could not next group due to invalid input stream state");
	}

	if (const auto grup_record_type = parse_record_type(input); grup_record_type != record_type_t::grup)
	{
		if (input.eof())
		{
			return no_next_grup;
		}
		return std::unexpected(
				std::format("Expected GRUP record type, found {} instead", to_record_string(grup_record_type))
		);
	}

	// For GRUPs, this data field also includes GRUP header size.
	const auto grup_total_size = parse_integral<record_size_t>(input);
	constexpr record_size_t grup_header_size = record_type_size + tes_size_of<record_size_t>() +
																						 (tes_size_of<std::uint8_t>() * 4U) + tes_size_of<std::uint32_t>() +
																						 (tes_size_of<std::uint16_t>() * 2U) + tes_size_of<std::uint32_t>();
	if (grup_total_size < grup_header_size)
	{
		return std::unexpected("Invalid GRUP size");
	}

	// Move input to the beginning of the data field of this grup.
	constexpr record_size_t grup_header_remaining_size =
			grup_header_size - record_type_size - tes_size_of<record_size_t>();
	seek_ahead(input, grup_header_remaining_size);

	const auto grup_data_size = grup_total_size - grup_header_size;
	if (grup_data_size == 0U)
	{
		// Report that this grup contains no records.
		return header_only_grup;
	}

	// Peek the contained record type only if the grup has a data field.
	const auto contained_record_type = parse_record_type(input);
	if (constexpr std::array invalid_contained_record_types{record_type_t::none, record_type_t::tes4};
			std::ranges::find(invalid_contained_record_types, contained_record_type) != invalid_contained_record_types.cend())
	{
		return std::unexpected(
				std::format("Invalid record type contained in group: {}", to_record_string(contained_record_type))
		);
	}

	// Return the stream to the start of the data field.
	input.seekg(input.tellg() - static_cast<std::ifstream::pos_type>(record_type_size));

	return grup_data_t{.record_type = contained_record_type, .data_size = grup_data_size};
}

std::expected<void, std::string> parse_group(
		std::ifstream& input, const grup_data_t grup_data, parsed_records_t& parsed_records
)
{
	const auto [contained_record_type, group_data_size] = grup_data;
	if (!input.good())
	{
		return std::unexpected(
				std::format(
						"Could not parse {} group due to invalid input stream state", to_record_string(contained_record_type)
				)
		);
	}

	const parse_record_data_func parse_data_func = get_parse_record_data_function(contained_record_type);
	if (parse_data_func == nullptr)
	{
		// This group data should not be parsed.
		seek_ahead(input, group_data_size);
		return {};
	}

	const auto group_data_end = input.tellg() + static_cast<std::ifstream::pos_type>(group_data_size);
	while (input.tellg() < group_data_end)
	{
		const auto header_result = parse_record_header(input);
		if (!header_result.has_value())
		{
			return std::unexpected(header_result.error());
		}
		const auto header_data = header_result.value();
		if (is_already_parsed(contained_record_type, header_data.record_id, parsed_records))
		{
			seek_ahead(input, header_data.data_size);
			continue;
		}
		const auto record_data_end = input.tellg() + static_cast<std::ifstream::pos_type>(header_data.data_size);
		if (auto parse_result = parse_data_func(input, header_data, parsed_records); !parse_result.has_value())
		{
			return parse_result;
		}
		input.seekg(record_data_end);

		if (!input.good())
		{
			return std::unexpected(
					std::format("Unexpected input stream state while parsing {} group", to_record_string(contained_record_type))
			);
		}
	}

	if (input.tellg() != group_data_end)
	{
		return std::unexpected(
				std::format(
						"Unexpected input stream position after parsing {} group. Expected was {:x}",
						to_record_string(contained_record_type), static_cast<std::int64_t>(group_data_end)
				)
		);
	}

	return {};
}

std::expected<void, std::string> close_tes_plugin(std::ifstream& input)
{
	if (!input.eof())
	{
		return std::unexpected("Attempting to close tes plugin that did not reach EOF");
	}

	input.close();
	return {};
}

}
