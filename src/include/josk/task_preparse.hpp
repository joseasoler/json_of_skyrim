#pragma once

#include <josk/tasks.hpp>

#include <expected>
#include <string>

namespace josk
{

/**
 * Preparses individual records.
 * @param task Information about the task to perform.
 * @return Task for merging the preparsed information, or an error.
 */
std::expected<task::task_merge_records, std::string> preparse_file(const task::task_preparse_records& task);

}
