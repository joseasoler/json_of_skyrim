#include <josk/tes_format.hpp>
#include <josk/tes_parse.hpp>

#include <strong_type/affine_point.hpp>
#include <strong_type/arithmetic.hpp>
#include <strong_type/formattable.hpp>
#include <strong_type/regular.hpp>
#include <strong_type/type.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "strong_type/ordered.hpp"

namespace josk::tes
{

/** Data used internally by the parser. */
struct parser
{
	/** Avoid using the stream instance directly. Only utility functions should interact with it. */
	std::ifstream input;
	/** Identifier for error reporting. */
	std::string_view name{"Invalid file name"};
	/** A pointer is used to avoid passing non-const references around. Null indicates non-initialized or an error. */
	parsed_records_t* records{};
};

}

namespace
{

using record_size_t = std::uint32_t;
using field_size_t = std::uint16_t;

/** Record, group or field ID, stored in string form. */
using section_str_id = std::array<char, josk::tes::section_id_byte_size>;

/** Represents differences between parser file positions. */
using offset_t =
		strong::type<std::int64_t, struct offset_t_, strong::regular, strong::arithmetic, strong::strongly_ordered>;
constexpr auto invalid_offset = std::numeric_limits<offset_t>::min();
constexpr auto section_str_id_offset = offset_t{josk::tes::section_id_byte_size};

template <typename type>
consteval offset_t offset_sizeof(const std::size_t count = 1Z)
{
	return static_cast<offset_t>(sizeof(type) * count);
}

/** Absolute parser file position. Negative values always indicate an error. */
using pos_t = strong::type<
		std::int64_t, struct parser_pos_t_, strong::regular, strong::affine_point<offset_t>, strong::formattable,
		strong::strongly_ordered>;
/** Marks an explicitly erroneous position. */
constexpr auto invalid_pos = std::numeric_limits<pos_t>::min();
/** Maximum position, represents end of file in reports. */
constexpr auto max_pos = std::numeric_limits<pos_t>::max();

constexpr offset_t record_header_size = section_str_id_offset + offset_sizeof<record_size_t>() +
																				offset_sizeof<std::uint32_t>() + offset_sizeof<josk::tes::formid_t>() +
																				offset_sizeof<std::uint16_t>(4Z);

struct group_data_t final
{
	josk::tes::record_type_t contained_record_type{josk::tes::record_type_t::none};
	offset_t data_size{invalid_offset};
	[[nodiscard]] bool operator==(const group_data_t&) const noexcept = default;
};

constexpr group_data_t invalid_group_data{};

struct record_header_data final
{
	josk::tes::formid_t record_id;
	offset_t data_size;
};

/**
 * RAII wrapper around the parser state, and definition of functions to interact with it.
 * Its life cycle is restricted to the task function that created it.
 */
class parser_impl final
{
public:
	using field_type_t = josk::tes::field_type_t;
	using formid_t = josk::tes::formid_t;
	using parser_state = josk::tes::parser;
	using records = josk::tes::parsed_records_t;
	using record_type_t = josk::tes::record_type_t;

private:
	/** Current parsing state. */
	std::unique_ptr<parser_state> _state;

public:
	/**
	 * Reconstruct the parser implementation using a previously allocated parser state.
	 * @param state Parser state.
	 */
	explicit parser_impl(parser_state* state);

	parser_impl(const parser_impl&) = delete;
	parser_impl(parser_impl&&) = default;
	parser_impl& operator=(const parser_impl&) = delete;
	parser_impl& operator=(parser_impl&&) = default;
	/** Held parser state memory will be automatically freed unless release is called. */
	~parser_impl() = default;

	enum class parser_status_t : std::uint8_t
	{
		valid = 0U,
		uninitialized,
		error,
		eof,
	};

	[[nodiscard]] parser_status_t get_status();

	/**
	 * Generates an error message for the current state. Cannot be const as querying stream position can modify it.
	 * @param description Short description of the error. Must start with lowercase and not end with a period.
	 * @return Formatted error message.
	 */
	[[nodiscard]] std::string error_message(std::string_view description);

	/**
	 * Releases the internal parser state from RAII management. Intended to pass the state to the next task.
	 * @return Pointer to the internal parser state.
	 */
	parser_state* release();

	/**
	 * Open the next record group. The parser must be at the beginning of the group.
	 * @return invalid_group_data if no more groups remain, valid group data otherwise. An error string if applicable.
	 */
	[[nodiscard]] std::expected<group_data_t, std::string> next_group();

	/**
	 * Helper function for retrieving an integral value of any type from the TES4 plugin file. Caller is responsible for
	 * managing the state of the input stream.
	 * @tparam integral_type Integral type being parsed.
	 * @return Parsed value if the operation went well, an undefined value otherwise.
	 */
	template <std::integral integral_type>
	[[nodiscard]] integral_type parse_integral()
	{
		integral_type value{};
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		_state->input.read(reinterpret_cast<char*>(&value), sizeof(integral_type));
		return value;
	}

	/**
	 * Parse individual records in a group.
	 * @param group_data Group data.
	 * @return Nothing, or an error.
	 */
	[[nodiscard]] std::expected<void, std::string> parse_group(group_data_t group_data);

	std::expected<record_header_data, std::string> parse_record_header(record_type_t record_type);

	std::expected<bool, std::string> parse_avif(josk::tes::formid_t record_id);
	/** Besides returning errors, parse functions may return false if the record has to be ignored. */
	using record_parse_func = std::expected<bool, std::string> (parser_impl::*)(josk::tes::formid_t);
	[[nodiscard]] static record_parse_func get_record_parse_func(record_type_t record_type) noexcept;

	[[nodiscard]] section_str_id parse_section_id();
	[[nodiscard]] record_type_t parse_record_type();
	[[nodiscard]] offset_t parse_record_size();
	[[nodiscard]] formid_t parse_formid();

	[[nodiscard]] offset_t parse_field_size();

	template <std::integral integral_type>
	[[nodiscard]] integral_type parse_integral_field_value()
	{
		return parse_integral<integral_type>();
	}

	[[nodiscard]] std::string parse_string_field_value(offset_t size)
	{
		std::string value(static_cast<std::string::size_type>(size.value_of()), '\0');
		_state->input.read(value.data(), size.value_of());
		return value;
	}

	[[nodiscard]] pos_t current_position();
	void seek_position(pos_t position);
	void seek_offset(offset_t offset);

	/**
	 * Parses the record type contained next in the stream and checks it against the provided value.
	 * The stream position will be moved to the end of the record type.
	 * @param record_type Record type to validate.
	 * @return True if the next record type contained in the stream was the requested type.
	 */
	[[nodiscard]] bool validate_record_type(record_type_t record_type);

	/**
	 * Parses the field type contained next in the stream and checks it against the provided value.
	 * The stream position will be moved to the end of the field type.
	 * @param field_type Field type to validate.
	 * @return True if the next field type contained in the stream was the requested type.
	 */
	[[nodiscard]] bool validate_field_type(field_type_t field_type);

	/**
	 * Skip the next field, depending on its type.
	 * @param field_type Field type to ignore.
	 */
	void ignore_field_if_present(field_type_t field_type);
};

parser_impl::parser_impl(parser_state* state)
	: _state{state}
{
}

[[nodiscard]] parser_impl::parser_status_t parser_impl::get_status()
{
	if (_state->records == nullptr)
	{
		return parser_status_t::uninitialized;
	}
	const auto& input = _state->input;
	if (input.eof())
	{
		return parser_status_t::eof;
	}
	if (input.good())
	{
		return parser_status_t::valid;
	}
	return parser_status_t::error;
}

std::string parser_impl::error_message(const std::string_view description)
{
	std::string_view stream_status{"error"};
	pos_t position{};
	switch (get_status())
	{
		case parser_status_t::valid:
			stream_status = "valid";
			position = current_position();
			break;
		case parser_status_t::uninitialized:
			stream_status = "uninitialized";
			position = invalid_pos;
			break;
		case parser_status_t::error:
			stream_status = "valid";
			position = current_position();
			break;
		case parser_status_t::eof:
			stream_status = "end of file";
			position = max_pos;
			break;
	}

	constexpr auto* format_str = "Parse error in {}: {}. State: {}, position: 0x{:x}.";
	return std::format(format_str, _state->name, description, stream_status, position);
}

std::expected<group_data_t, std::string> parser_impl::next_group()
{
	if (get_status() != parser_status_t::valid)
	{
		return std::unexpected(error_message("invalid file stream state before opening next record group"));
	}

	if (!validate_record_type(record_type_t::grup))
	{
		if (get_status() == parser_status_t::eof)
		{
			return invalid_group_data;
		}
		return std::unexpected(error_message("missing expected GRUP header"));
	}

	// In GRUP headers, the data size field includes GRUP header size.
	const auto total_grup_size = parse_record_size();
	constexpr offset_t grup_header_remaining_size =
			offset_sizeof<std::uint32_t>() + offset_sizeof<josk::tes::formid_t>() + offset_sizeof<std::uint16_t>(4Z);

	constexpr offset_t grup_header_total_size =
			section_str_id_offset + offset_sizeof<record_size_t>() + grup_header_remaining_size;
	if (total_grup_size < grup_header_total_size)
	{
		return std::unexpected(error_message("Invalid GRUP size"));
	}
	// Seek to the end of the header of the current GRUP.
	seek_offset(grup_header_remaining_size);
	if (total_grup_size == grup_header_total_size)
	{
		// This is an empty group with no records.
		// Zero data size and an invalid record type will skip data parsing.
		return group_data_t{.contained_record_type = record_type_t::grup, .data_size = offset_t{0U}};
	}

	// Peek the header of the first contained record to find its record type id.
	const auto contained_record_type = parse_record_type();
	// Return the stream to the start of the header of the first record.
	seek_offset(-section_str_id_offset);
	return group_data_t{
			.contained_record_type = contained_record_type, .data_size = total_grup_size - grup_header_total_size
	};
}

std::expected<void, std::string> parser_impl::parse_group(const group_data_t group_data)
{
	const auto& [contained_record_type, group_data_size] = group_data;
	if (get_status() != parser_status_t::valid)
	{
		const auto formatted_error = std::format(
				"invalid file stream state while parsing {} record group", josk::tes::to_record_string(contained_record_type)
		);
		return std::unexpected(error_message(formatted_error));
	}

	const pos_t group_data_end = current_position() + group_data_size;

	const auto parse_func = get_record_parse_func(contained_record_type);
	if (parse_func == nullptr)
	{
		// Group that does not require parsing.
		seek_position(group_data_end);
	}

	while (current_position() < group_data_end)
	{
		auto parse_header_result = parse_record_header(contained_record_type);
		if (!parse_header_result.has_value())
		{
			return std::unexpected(parse_header_result.error());
		}

		const auto& [record_type, record_data_size] = parse_header_result.value();

		const auto record_data_end = current_position() + record_data_size;

		if (auto& parsed_formids = _state->records->parsed_formids; !parsed_formids.contains(record_type))
		{
			const auto parse_record_data_result = std::invoke(parse_func, *this, record_type);
			if (!parse_record_data_result.has_value())
			{
				return std::unexpected(parse_record_data_result.error());
			}
			if (parse_record_data_result.value())
			{
				parsed_formids.emplace(record_type);
			}
		}
		seek_position(record_data_end);
	}

	if (current_position() != group_data_end)
	{
		const auto formatted_error = std::format("group parsing did not reach expected end position {}", group_data_end);
		return std::unexpected(error_message(formatted_error));
	}

	return {};
}

std::expected<record_header_data, std::string> parser_impl::parse_record_header(const record_type_t record_type)
{
	// Record type is known.
	if (!validate_record_type(record_type))
	{
		return std::unexpected(error_message("unexpected record type while parsing group"));
	}
	record_header_data header_data{};
	header_data.data_size = parse_record_size();
	// Flags are currently not required by josk.
	seek_offset(offset_sizeof<std::uint32_t>());
	header_data.record_id = parse_formid();
	constexpr offset_t remaining_header_size_after_formid = record_header_size - section_str_id_offset -
																													offset_sizeof<record_size_t>() -
																													offset_sizeof<std::uint32_t>() - offset_sizeof<formid_t>();
	seek_offset(remaining_header_size_after_formid);
	if (!_state->input.good())
	{
		return std::unexpected(error_message("invalid file stream state during record header parsing"));
	}

	return header_data;
}

std::expected<bool, std::string> parser_impl::parse_avif(const josk::tes::formid_t record_id)
{
	if (!validate_field_type(field_type_t::edid))
	{
		return false;
	}
	seek_offset(parse_field_size());

	if (!validate_field_type(field_type_t::full))
	{
		return false;
	}
	[[maybe_unused]] const auto name_size = parse_field_size();
	[[maybe_unused]] const auto name_position = current_position();
	seek_offset(name_size);

	if (!validate_field_type(field_type_t::desc))
	{
		return false;
	}
	[[maybe_unused]] const auto description_size = parse_field_size();
	[[maybe_unused]] const auto description_position = current_position();
	seek_offset(description_size);

	// Some AVIFs such as one-handed and two-handed have an ANAM field.
	ignore_field_if_present(field_type_t::anam);

	if (!validate_field_type(field_type_t::cnam))
	{
		return false;
	}
	seek_offset(offset_sizeof<field_size_t>());
	const auto cnam_value = parse_integral_field_value<std::uint32_t>();
	if (constexpr auto max_cnam_value = static_cast<std::uint32_t>(josk::tes::skill_category_t::stealth);
			max_cnam_value < cnam_value)
	{
		return false;
	}

	// ToDo missing two handed and one handed

	auto& avif_record = _state->records->avif_records.emplace_back();
	avif_record.record_id = record_id;
	avif_record.category = static_cast<josk::tes::skill_category_t>(cnam_value);
	seek_position(name_position);
	avif_record.name = parse_string_field_value(name_size);
	seek_position(description_position);
	avif_record.description = parse_string_field_value(description_size);

	return true;
}

parser_impl::record_parse_func parser_impl::get_record_parse_func(const record_type_t record_type) noexcept
{
	switch (record_type)
	{
		case record_type_t::avif:
			return &parser_impl::parse_avif;
		default:
			break;
	}
	return nullptr;
}

parser_impl::parser_state* parser_impl::release()
{
	return _state.release();
}

section_str_id parser_impl::parse_section_id()
{
	section_str_id section_id;
	_state->input.read(section_id.data(), section_id.size());
	return _state->input.good() ? section_id : section_str_id{};
}

parser_impl::record_type_t parser_impl::parse_record_type()
{
	const auto section_id = parse_section_id();
	return josk::tes::to_record_type(std::string_view(section_id.data(), section_id.size()));
}

offset_t parser_impl::parse_record_size()
{
	const auto record_size = parse_integral<record_size_t>();
	return _state->input.good() ? offset_t{record_size} : invalid_offset;
}

parser_impl::formid_t parser_impl::parse_formid()
{
	const auto formid = parse_integral<formid_t>();
	return _state->input.good() ? formid : formid_t{};
}

offset_t parser_impl::parse_field_size()
{
	const auto field_size = parse_integral<field_size_t>();
	return _state->input.good() ? offset_t{field_size} : invalid_offset;
}

pos_t parser_impl::current_position()
{
	return pos_t{_state->input.tellg()};
}

void parser_impl::seek_position(const pos_t position)
{
	_state->input.seekg(static_cast<std::ifstream::pos_type>(position.value_of()));
}

void parser_impl::seek_offset(const offset_t offset)
{
	_state->input.seekg(static_cast<std::ifstream::off_type>(offset.value_of()), std::ios_base::cur);
}

bool parser_impl::validate_record_type(const record_type_t record_type)
{
	const auto record_string = josk::tes::to_record_string(record_type);
	const auto parsed_string = parse_section_id();
	return std::ranges::equal(record_string, parsed_string);
}

bool parser_impl::validate_field_type(const field_type_t field_type)
{
	const auto field_string = josk::tes::to_field_string(field_type);
	const auto parsed_string = parse_section_id();
	return std::ranges::equal(field_string, parsed_string);
}

void parser_impl::ignore_field_if_present(const field_type_t field_type)
{
	if (validate_field_type(field_type))
	{
		seek_offset(parse_field_size());
	}
	else
	{
		// Record not found. Restore the stream to its previous position.
		seek_offset(-section_str_id_offset);
	}
}

std::expected<parser_impl, std::string> acquire_state(josk::tes::parser* parser_ptr)
{
	assert(parser_ptr != nullptr);
	return parser_impl{parser_ptr};
}

/**
 * Open a plugin file. The parser must not have opened a file already.
 * @param path Path to existing regular file that should be a plugin file.
 * @param name File name identifier used as an identifier on reports.
 * @param records Data structure holding parsed records from previous plugin files.
 * @return Parser, or an error.
 */
std::expected<parser_impl, std::string> open(
		const std::filesystem::path& path, const std::string_view name, parser_impl::records& records
)
{
	auto parser_ptr = std::make_unique<josk::tes::parser>();
	parser_ptr->name = name;
	parser_ptr->records = &records;

	constexpr auto open_flags = static_cast<std::ios_base::openmode>(
			static_cast<unsigned int>(std::ios::binary) | static_cast<unsigned int>(std::ios::in)
	);
	parser_ptr->input.open(path, open_flags);
	parser_impl parser{parser_ptr.release()};
	if (parser.get_status() != parser_impl::parser_status_t::valid)
	{
		return std::unexpected(parser.error_message("could not open file"));
	}

	if (!parser.validate_record_type(parser_impl::record_type_t::tes4))
	{
		return std::unexpected(parser.error_message("invalid TES4 file"));
	}

	const auto tes4_data_size = parser.parse_record_size();
	// Remaining header size after parsing record type and data size.
	constexpr offset_t remaining_header_size_after_data =
			record_header_size - section_str_id_offset - offset_sizeof<record_size_t>();
	const auto tes4_record_end_offset = tes4_data_size + remaining_header_size_after_data;
	parser.seek_offset(tes4_record_end_offset);

	return parser;
}

std::expected<parser_impl, std::string> parse(parser_impl impl)
{
	auto next_group_result = impl.next_group();
	while (next_group_result.has_value() && next_group_result.value() != invalid_group_data)
	{
		if (auto parse_group_result = impl.parse_group(next_group_result.value()); !parse_group_result.has_value())
		{
			return std::unexpected(parse_group_result.error());
		}

		next_group_result = impl.next_group();
	}

	if (!next_group_result.has_value())
	{
		return std::unexpected(next_group_result.error());
	}

	return impl;
}

std::expected<void, std::string> close(parser_impl impl)
{
	if (impl.get_status() != parser_impl::parser_status_t::eof)
	{
		return std::unexpected(impl.error_message("closing file that did not finish parsing"));
	}

	return {};
}

/**
 * Releases the internal parser state from RAII management. Intended to pass the state to the next task.
 * @return Pointer to the internal parser state.
 */
std::expected<josk::tes::parser*, std::string> release(parser_impl impl)
{
	return impl.release();
}

}

namespace josk::tes
{
std::expected<parser*, std::string> open_plugin(
		const std::filesystem::path& path, const std::string_view filename, parsed_records_t& parsed_records
)
{
	return open(path, filename, parsed_records).and_then(release);
}

std::expected<parser*, std::string> parse_plugin(parser* parser_ptr)
{
	return acquire_state(parser_ptr).and_then(parse).and_then(release);
}

std::expected<void, std::string> close_plugin(parser* parser_ptr)
{
	return acquire_state(parser_ptr).and_then(close);
}

}
