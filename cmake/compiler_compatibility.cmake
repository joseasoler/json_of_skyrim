include_guard(GLOBAL)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(JOSK_CXX_COMPILER_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(JOSK_CXX_COMPILER_GCC 1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(JOSK_CXX_COMPILER_MSVC 1)
else()
	message(WARNING "Compiler ${CMAKE_CXX_COMPILER_VERSION} is not supported by ${CMAKE_PROJECT_NAME}.")
endif ()
