#pragma once

#include <josk/tes_format.hpp>

#include <expected>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace josk::tes
{

/**
 * The data vector contains unparsed group data for all the records of that record type.
 * The first record type in the data vector is missing its record_type.
 */
using raw_record_groups = std::unordered_map<record_type, std::vector<char>>;

/**
 * Gathers record groups matching the provided types. Records are not parsed at this stage.
 * @param path Path pointing to the file.
 * @param requested_record_types Record types to gather.
 * @return Map of requested record groups, or an error.
 */
std::expected<raw_record_groups, std::string> read_file(
		const char* path, const std::unordered_set<record_type>& requested_record_types
);

}
