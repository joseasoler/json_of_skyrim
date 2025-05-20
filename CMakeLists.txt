cmake_minimum_required(VERSION 4.0)

project(json_of_skyrim
	VERSION 0.1.0
	DESCRIPTION "Generate JSON data from a Skyrim modlist."
	HOMEPAGE_URL "https://github.com/joseasoler/json_of_skyrim"
	LANGUAGES C CXX
)

# The project uses C++23 without any non-standard compiler extensions.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check compiler compatibility.
include(cmake/compiler_compatibility.cmake)

# Set common target compile options and definitions for josk targets.
set(JOSK_CXX_COMPILE_OPTIONS)
set(JOSK_CXX_COMPILE_DEFINITIONS)
include(cmake/compiler_options.cmake)
include(cmake/compiler_warnings.cmake)

# Tooling.
include(cmake/clang_format.cmake)
include(cmake/clang_tidy.cmake)

add_subdirectory(src)
