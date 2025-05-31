#pragma once

#include <josk/tes_format.hpp>

#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

namespace josk::tes
{

/** Parsed records gathered from all modlist plugins following load order rules. */
struct parsed_records_t final
{
	std::unordered_map<formid_t, avif_record> avif_records;
};

/**
 * Opens a TES plugin file and initializes its parsing data.
 * @param path File path to open.
 * @return TES plugin file input stream.
 */
std::expected<std::ifstream, std::string> open_tes_plugin(const std::filesystem::path& path);

struct grup_data_t final
{
	/** Record type contained by this group. Guaranteed to not be NONE, GRUP (except for header only groups) or TES4. */
	record_type_t record_type{};
	/** Data field size for this group. Will be zero for header-only groups. */
	record_size_t data_size{};
	bool operator==(const grup_data_t& grup_data) const = default;
};

constexpr grup_data_t no_next_grup{.record_type = record_type_t::none, .data_size = 0U};
constexpr grup_data_t header_only_grup{.record_type = record_type_t::grup, .data_size = 0U};

/**
 * Seek to the data field of the current group in the TES file.
 * @param input TES plugin file input stream. Assumed to be right at the beginning of a group header.
 * @return Contained group record type and total size.
 */
std::expected<grup_data_t, std::string> open_next_group(std::ifstream& input);

/**
 * Parse the next group in the TES file.
 * @param input TES plugin file input stream. Expected to point to the header of the first record in the group.
 * @param grup_data Record type found in the group and total data size.
 * @param parsed_records Data structure to store any new parsed records.
 * @return Error if any, otherwise nothing.
 */
std::expected<void, std::string> parse_group(
		std::ifstream& input, grup_data_t grup_data, parsed_records_t& parsed_records
);

/**
 * Close a TES plugin parser and retrieve its records after parsing is finished.
 * @param input TES plugin file input stream.
 * @return Error if any, otherwise nothing.
 */
std::expected<void, std::string> close_tes_plugin(std::ifstream& input);

}
