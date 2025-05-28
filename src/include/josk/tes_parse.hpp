#pragma once

#include <josk/tes_format.hpp>

#include <concepts>
#include <expected>
#include <fstream>
#include <string>
#include <unordered_set>

namespace josk::tes
{

const std::unordered_set<record_type_t>& required_records();

template <std::integral integral_type>
integral_type parse_integral(std::ifstream& input)
{
	integral_type value{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	input.read(reinterpret_cast<char*>(&value), sizeof(value));
	return value;
}

record_type_t parse_record_type(std::ifstream& input);

void seek_ahead(std::ifstream& input, tes_size_t jump_size);

std::expected<void, std::string> parse_tes4_record(std::ifstream& input);

}
