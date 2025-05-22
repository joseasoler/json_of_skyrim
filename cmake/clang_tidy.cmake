include_guard(GLOBAL)

option(JOSK_CLANG_TIDY "Analyze the project with clang-tidy" OFF)

if (JOSK_CLANG_TIDY)
	find_program(
		JOSK_CLANG_TIDY_BINARY
		NAMES "clang-tidy"
		DOC "Clang-tidy binary"
	)

	if (JOSK_CLANG_TIDY_BINARY)
		set(JOSK_CLANG_TIDY_OPTIONS
			${JOSK_CLANG_TIDY_BINARY}
			-extra-arg=-Wno-unknown-warning-option
			-extra-arg=-Wno-ignored-optimization-argument
			-extra-arg=-Wno-unused-command-line-argument
			-extra-arg=-DJOSK_CLANG_TIDY
			-extra-arg=/EHsc
		)

		if (CMAKE_COMPILE_WARNING_AS_ERROR)
			list(APPEND JOSK_CLANG_TIDY_OPTIONS -warnings-as-errors=*)
		endif ()

		set(CMAKE_CXX_CLANG_TIDY ${JOSK_CLANG_TIDY_OPTIONS})
	else ()
		message(FATAL_ERROR "JOSK_CLANG_TIDY is enabled but the clang-tidy binary was not found.")
	endif ()
endif ()
