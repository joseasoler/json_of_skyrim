#include <josk/task_load.hpp>
#include <josk/tasks.hpp>
#include <josk/tes_format.hpp>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace
{

using josk::tes::record_type;
using josk::tes::record_type_size;
using josk::tes::tes_size_of;
using josk::tes::tes_size_t;

const std::unordered_set<record_type>& requested_records()
{
	static const std::unordered_set records{record_type::avif, record_type::perk};
	return records;
}

template <std::integral integral_type>
integral_type read_integral(std::ifstream& input)
{
	integral_type value{};
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	input.read(reinterpret_cast<char*>(&value), sizeof(value));
	return value;
}

record_type read_record_type(std::ifstream& input)
{
	return static_cast<record_type>(read_integral<std::underlying_type_t<record_type>>(input));
}

void jump_ahead(std::ifstream& input, const tes_size_t jump_size)
{
	input.seekg(input.tellg() + static_cast<std::ifstream::pos_type>(jump_size));
}

}

namespace josk
{

std::expected<task::task_preparse_records, std::string> load_file(const task::task_load_file& task)
{
	if (const auto& path = task.path; !std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
	{
		return std::unexpected(std::format("Could not find file {}.", task.filename));
	}

	constexpr auto open_flags = static_cast<std::ios_base::openmode>(
			static_cast<unsigned int>(std::ios::binary) | static_cast<unsigned int>(std::ios::in)
	);

	std::ifstream input(task.path, open_flags);
	if (!input.is_open() || !input.good())
	{
		return std::unexpected(std::format("Could not open file {}.", task.filename));
	}

	if (const auto tes4_record_type = read_record_type(input); tes4_record_type != record_type::tes4 || !input.good())
	{
		return std::unexpected(std::format("{} is not a TES4 file", task.filename));
	}

	// TES4 record header size excluding the record type and data size fields.
	constexpr tes_size_t record_header_remaining_size =
			tes::record_header_size - record_type_size - tes_size_of<tes_size_t>();

	const tes_size_t tes4_record_remaining_size = read_integral<tes_size_t>(input) + record_header_remaining_size;
	jump_ahead(input, tes4_record_remaining_size);

	// Read the different group records.
	task::raw_record_groups record_groups{};
	auto grup_record_type = read_record_type(input);
	while (!input.eof())
	{
		if (grup_record_type != record_type::grup || !input.good())
		{
			const auto position = static_cast<std::int64_t>(input.tellg());
			return std::unexpected(
					std::format("Error during GRUP processing at position {} of file {}.", position, task.filename)
			);
		}

		constexpr tes_size_t grup_header_remaining_size =
				tes::group_header_size - record_type_size - tes_size_of<tes_size_t>();

		// Unlike records, the GRUP data field includes the header size.
		const auto grup_total_size = read_integral<tes_size_t>(input);

		jump_ahead(input, grup_header_remaining_size);

		if (grup_total_size == tes::group_header_size)
		{
			// Header-only group without data. The reader is already pointing to the next group. Skip and carry on.
			grup_record_type = read_record_type(input);
			continue;
		}

		const tes_size_t grup_data_size = grup_total_size - tes::group_header_size;
		const tes_size_t grup_remaining_data_size = grup_data_size - record_type_size;

		if (const auto grup_contained_record_type = read_record_type(input);
				requested_records().contains(grup_contained_record_type))
		{
			// josk assumes that a single file never has more than one group of the same record type.
			assert(!record_groups.contains(grup_contained_record_type));
			auto& record_group_data = record_groups[grup_contained_record_type];
			record_group_data.resize(grup_data_size);
			// Manually copy the first record type to the start of the data.
			std::memcpy(record_group_data.data(), &grup_contained_record_type, record_type_size);
			// Read the remaining data directly into the array.
			input.read(record_group_data.data() + record_type_size, grup_remaining_data_size);
		}
		else
		{
			jump_ahead(input, grup_remaining_data_size);
		}

		// Read the record type of the group to be processed in the next iteration.
		grup_record_type = read_record_type(input);
	}

	input.close();

	task::task_preparse_records new_task{};
	new_task.priority = task.priority;
	new_task.filename = task.filename;
	new_task.groups = std::move(record_groups);
	return new_task;
}

}
