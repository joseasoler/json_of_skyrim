#pragma once

#include <josk/tasks.hpp>

#include <expected>
#include <string>

namespace josk
{

/**
 * Gathers record groups matching the provided types. Records are not parsed at this stage.
 * @param task Information about the task to perform.
 * @return Task for preparsing the loaded information, or an error.
 */
std::expected<task::task_preparse_records, std::string> load_file(const task::task_load_file& task);

}
