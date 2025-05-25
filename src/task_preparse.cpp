#include <josk/task_preparse.hpp>
#include <josk/tasks.hpp>
#include <josk/tes_format.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{

using namespace josk::task;
using namespace josk::tes;

std::expected<std::size_t, std::string> preparse_record(
		const std::vector<char>& group_data, preparsed_record_group& records, const std::size_t start_index
)
{
	// Type is assumed to be correct.
	const auto data_size_index = start_index + record_type_size;

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	const tes_size_t record_data_size = reinterpret_cast<const tes_size_t&>(group_data[data_size_index]);
	const auto formid_index = data_size_index + tes_size_of<tes_size_t>() + sizeof(std::uint32_t);

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	const formid_t form_id = reinterpret_cast<const formid_t&>(group_data[formid_index]);
	const std::size_t after_end_index = start_index + record_header_size + record_data_size;

	if (records.contains(form_id))
	{
		return std::unexpected(std::format("Duplicate form id {}", form_id));
	}

	auto& record_data = records[form_id];
	record_data.resize(record_data_size);
	const auto group_begin_itr = group_data.cbegin() + static_cast<std::ptrdiff_t>(start_index + record_header_size);
	const auto group_end_itr = group_data.cbegin() + static_cast<std::ptrdiff_t>(after_end_index);
	std::copy(group_begin_itr, group_end_itr, record_data.begin());

	return after_end_index;
}

std::expected<preparsed_record_group, std::string> preparse_group(const std::vector<char>& group_data)
{
	preparsed_record_group records{};

	std::size_t group_data_index{0Z};
	while (group_data_index < group_data.size())
	{
		auto preparse_record_result = preparse_record(group_data, records, group_data_index);
		if (!preparse_record_result.has_value())
		{
			return std::unexpected(preparse_record_result.error());
		}
		group_data_index = preparse_record_result.value();
	}

	return records;
}

}

namespace josk
{

std::expected<task_merge_records, std::string> preparse_file(const task_preparse_records& task)
{
	task_merge_records result{};

	for (const auto& [record_type, group_data] : task.groups)
	{
		auto preparse_group_result = preparse_group(group_data);
		if (!preparse_group_result.has_value())
		{
			return std::unexpected(preparse_group_result.error());
		}
		result.groups[record_type] = preparse_group_result.value();
	}

	return result;
}

}
