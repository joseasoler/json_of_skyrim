# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include_guard(GLOBAL)

if (JOSK_CXX_COMPILER_CLANG)
	list(APPEND JOSK_CXX_COMPILE_DEFINITIONS
		JOSK_COMPILER_CLANG=1
		JOSK_COMPILER_GCC=0
		JOSK_COMPILER_MSVC=0
	)
elseif (JOSK_CXX_COMPILER_GCC)
	list(APPEND JOSK_CXX_COMPILE_DEFINITIONS
		JOSK_COMPILER_CLANG=0
		JOSK_COMPILER_GCC=1
		JOSK_COMPILER_MSVC=0
	)
elseif (JOSK_CXX_COMPILER_MSVC)
	list(APPEND JOSK_CXX_COMPILE_DEFINITIONS
		JOSK_COMPILER_CLANG=0
		JOSK_COMPILER_GCC=0
		JOSK_COMPILER_MSVC=1
		# In case the project ever needs to access the Windows API directly.
		NOMINMAX
		WIN32_LEAN_AND_MEAN
	)
	# UTF-8 support is applied globally. See UTF-8 Everywhere for details.
	add_compile_definitions(UNICODE _UNICODE)
	add_compile_options(/utf-8)
	# Enable preprocessor conformance mode.
	add_compile_options(/Zc:preprocessor)
else ()
	list(APPEND JOSK_CXX_COMPILE_DEFINITIONS
		JOSK_COMPILER_CLANG=0
		JOSK_COMPILER_GCC=0
		JOSK_COMPILER_MSVC=0
	)
endif ()

