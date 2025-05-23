#pragma once

#include <josk/tasks.hpp>

#include <expected>

namespace josk
{

std::expected<task::load_order_data, std::string> parse_load_order(const std::filesystem::path& load_order_path);

}
