add_executable(josk
		cli.cpp
		josk.cpp
		task_find_plugins.cpp
		task_parse_load_order.cpp
		task_parse_plugins.cpp
		tes_format.cpp
		tes_parse.cpp
		${PROJECT_SOURCE_DIR}/josk_application.manifest
)

target_include_directories(josk PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/include
)

target_compile_definitions(josk PRIVATE ${JOSK_CXX_COMPILE_DEFINITIONS})
target_compile_options(josk PRIVATE ${JOSK_CXX_COMPILE_OPTIONS})

target_link_libraries(josk PRIVATE
		CLI11::CLI11
		strong_type::strong_type
)

if (JOSK_CLANG_FORMAT_BINARY)
	add_dependencies(josk josk_clang_format)
endif ()

install(TARGETS josk RUNTIME)
