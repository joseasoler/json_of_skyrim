#pragma once

#include <josk/tes_format.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace josk::task
{

/** Priority of a task. Corresponds to the load order of the file. */
using priority_t = std::int64_t;
constexpr priority_t invalid_priority{-1};

/** Maps each load order filename with its priority. */
using load_order_data = std::unordered_map<std::string, priority_t>;

/** Find plugins in the filesystem. */
/*
struct search_plugins_task final
{
	load_order_data load_order;
	std::filesystem::path skyrim_data_path;
	// std::filesystem::path mods_path;
};
*/

/** Common fields for tasks related to an individual file. */
struct file_task
{
	/** Tasks with lower priorities will be undertaken first. */
	priority_t priority{invalid_priority};
	/** Plugin filename. */
	std::string_view filename;
};

/** Load a file from the filesystem. */
struct load_task final : file_task
{
	std::filesystem::path path;
};

/** Maps a contained record type to raw group data storing all records of that type. */
using raw_record_groups = std::unordered_map<tes::record_type, std::vector<char>>;

/** Split groups into individual records, identifying them by formid. */
struct preparse_task final : file_task
{
	raw_record_groups groups;
};

/*
struct preparsed_record final
{
	tes::formid_t id{};
	std::vector<char> data;
};

using preparsed_record_groups = std::unordered_map<tes::record_type, std::vector<preparsed_record>>;

// Records ready to be merged.
struct merge_task final : file_task
{
	preparsed_record_groups groups;
};
*/

}
