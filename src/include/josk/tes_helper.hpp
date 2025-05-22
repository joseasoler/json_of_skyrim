#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace josk::tes
{

/**
 * Convert string representations of record types such as TES4 or GRUP to a number at compile time.
 * @param str Record type as a string. Must have a size of exactly 4.
 * @return Record type as an unsigned integer.
 */
consteval std::uint32_t to_record_type(const std::string_view str)
{
	if (constexpr std::size_t record_type_size = sizeof(std::uint32_t); str.size() != record_type_size)
	{
		throw std::invalid_argument("Record type strings must have a size of 4.");
	}

	std::uint32_t result{};
	for (int char_index = 3; char_index >= 0; --char_index)
	{
		result <<= CHAR_BIT;
		result |= static_cast<std::uint32_t>(str[static_cast<std::size_t>(char_index)]);
	}

	return result;
}

}
