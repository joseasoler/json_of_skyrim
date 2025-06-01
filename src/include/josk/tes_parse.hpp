#pragma once

#include <josk/tes_format.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

namespace josk::tes
{

/** Parsed records gathered from all modlist plugins following load order rules. */
struct parsed_records_t final
{
	std::unordered_map<formid_t, avif_record> avif_records;
};

/** TES file parser implementation. Must be passed to the next file parsing task and not handled outside of them. */
struct parser;

/**
 * Opens a TES plugin file with a parser. parsed_records must exist for the entire parser lifetime.
 * @param path Path to existing regular file that should be a plugin file.
 * @param filename File name identifier used as an identifier on reports.
 * @param parsed_records Records parsed on plugins with higher load order than this one.
 * @return TES plugin parser.
 */
std::expected<parser*, std::string> open_plugin(
		const std::filesystem::path& path, std::string_view filename, parsed_records_t& parsed_records
);

/**
 * Parse all required records and place them in the parsed records data structure.
 * @param parser_ptr TES plugin parser.
 * @return TES plugin parser or error.
 */
std::expected<parser*, std::string> parse_plugin(parser* parser_ptr);

/**
 * Validate and close a TES plugin parser.
 * @param parser_ptr TES plugin parser.
 * @return Error if any, otherwise nothing. Records are placed directly into parsed_records by previous stages.
 */
std::expected<void, std::string> close_plugin(parser* parser_ptr);

}
