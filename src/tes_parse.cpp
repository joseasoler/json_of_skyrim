#include <josk/tes_format.hpp>
#include <josk/tes_parse.hpp>

#include <expected>
#include <fstream>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace josk::tes
{

const std::unordered_set<record_type_t>& required_records()
{
	static const std::unordered_set records{record_type_t::avif};
	return records;
}

record_type_t parse_record_type(std::ifstream& input)
{
	return static_cast<record_type_t>(parse_integral<std::underlying_type_t<record_type_t>>(input));
}

void seek_ahead(std::ifstream& input, const tes_size_t jump_size)
{
	input.seekg(input.tellg() + static_cast<std::ifstream::pos_type>(jump_size));
}

std::expected<void, std::string> parse_tes4_record(std::ifstream& input)
{
	if (const auto tes4_record_type = parse_record_type(input); tes4_record_type != record_type_t::tes4 || !input.good())
	{
		return std::unexpected("Could not parse TES4 record.");
	}
	// Remaining header size after parsing record type and data size.
	constexpr tes_size_t header_remaining_size = record_header_size - record_type_size - tes_size_of<tes_size_t>();
	seek_ahead(input, parse_integral<tes_size_t>(input) + header_remaining_size);
	return {};
}

}
