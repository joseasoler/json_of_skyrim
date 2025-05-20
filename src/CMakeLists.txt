add_executable(josk
	josk.cpp
	${PROJECT_SOURCE_DIR}/josk_application.manifest
)

target_compile_definitions(josk PRIVATE ${JOSK_CXX_COMPILE_DEFINITIONS})
target_compile_options(josk PRIVATE ${JOSK_CXX_COMPILE_OPTIONS})

if (JOSK_CLANG_FORMAT_BINARY)
	add_dependencies(josk josk_clang_format)
endif ()

install(TARGETS josk RUNTIME)


