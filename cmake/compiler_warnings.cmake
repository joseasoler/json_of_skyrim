include_guard(GLOBAL)

if (JOSK_CXX_COMPILER_CLANG OR JOSK_CXX_COMPILER_GCC)
	# Warnings present in all supported versions of GCC and Clang.
	list(APPEND JOSK_CXX_COMPILE_OPTIONS
		-Wall                # Enables most warnings.
		-Wextra              # Enables an extra set of warnings.
		-pedantic            # Strict compliance to the standard is not met.
		-Wconversion         # Implicit type conversions that may change a value.
		-Wdate-time          # Macros that might prevent bit-wise-identical compilations are encountered.
		-Wdouble-promotion   # Implicit conversions from "float" to "double".
		-Wextra-semi         # Redundant semicolons after in-class function definitions.
		-Wformat=2           # printf/scanf/strftime/strfmon format string anomalies.
		-Wnull-dereference   # Dereferencing a pointer may lead to erroneous or undefined behavior.
		-Wold-style-cast     # C-style cast is used in a program.
		-Woverloaded-virtual # Overloaded virtual function names.
		-Wsign-conversion    # Implicit conversions between signed and unsigned integers.
		-Wshadow             # One variable shadows another.
		-Wsuggest-override   # Overriding virtual functions that are not marked with the override keyword.
		-Wswitch-enum        # A switch statement has an index of enumerated type and lacks a case.
		-Wundef              # An undefined identifier is evaluated in an #if directive.
	)
	# Enable additional warnings depending on the compiler type and version in use.
	if (JOSK_CXX_COMPILER_GCC)
		list(APPEND JOSK_CXX_COMPILE_OPTIONS
			-Wcast-align=strict         # Pointer casts which increase alignment.
			-Wdisabled-optimization     # A requested optimization pass is disabled.
			-Wduplicated-branches       # Duplicated branches in if-else statements.
			-Wduplicated-cond           # Duplicated conditions in an if-else-if chain.
			-Wlogical-op                # A logical operator is always evaluating to true or false.
			-Wredundant-decls           # Something is declared more than once in the same scope.
			-Wunsafe-loop-optimizations # A loop cannot be optimized.
			-Wuseless-cast              # Useless casts.
		)
	endif ()
elseif (JOSK_CXX_COMPILER_MSVC)
	list(APPEND JOSK_CXX_COMPILE_OPTIONS
		/permissive- # Specify standards conformance mode to the compiler.
		/W4          # Enable level 4 warnings.
		/analyze     # Code analysis.
		# Warnings triggered by external headers.
		/wd6239      # fmt.
	)
endif ()
