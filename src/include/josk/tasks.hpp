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

/** Common fields for tasks related to an individual file. */
struct task_file
{
	/** Tasks with lower priorities will be undertaken first. */
	priority_t priority{invalid_priority};
	/** Plugin filename. */
	std::string filename;
};

/** Load a file from the filesystem. */
struct task_load_file final : task_file
{
	std::filesystem::path path;
};

/** Maps a contained record type to raw group data storing all records of that type. */
using raw_record_groups = std::unordered_map<tes::record_type, std::vector<char>>;

/** Split groups into individual records, identifying them by formid. */
struct task_preparse_records final : task_file
{
	raw_record_groups groups;
};

using preparsed_record_groups =
		std::unordered_map<tes::record_type, std::unordered_map<tes::formid_t, std::vector<char>>>;

/*
// Records ready to be merged.
struct task_merge_records final : task_file
{
	preparsed_record_groups groups;
};
*/

}
