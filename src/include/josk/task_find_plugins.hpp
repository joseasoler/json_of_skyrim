#pragma once

#include <josk/cli.hpp>
#include <josk/tasks.hpp>

#include <expected>
#include <string>
#include <vector>

namespace josk
{

// ToDo: currently works only on single thread.
std::expected<std::vector<task::task_load_file>, std::string> find_plugins(const cli::args& arguments);

}
