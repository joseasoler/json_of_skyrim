# Contributing to JSON of Skyrim

Thank you for being interested on contributing to this project!

## Questions and bug reports

You can use the [issue tracker](https://github.com/joseasoler/json_of_skyrim/issues) to ask questions and report bugs. Please use the search (including closed issues) to see if your entry has been discussed before.

## Contributions

Before submitting pull requests for new features, create an issue on the [tracker](https://github.com/joseasoler/json_of_skyrim/issues) to allow discussing and refining the idea before it is implemented.

### Source code contributions

Enable the following CMake options: `CMAKE_COMPILE_WARNING_AS_ERROR`, `JOSK_CLANG_TIDY`. Alternatively, use one of the provided CMake presets.

In both cases, [clang-tidy](https://clang.llvm.org/extra/clang-tidy) and [clang-format](https://clang.llvm.org/docs/ClangFormat.html) must be installed. Most of the [style guide](STYLE_GUIDE.md) rules are checked automatically by clang-format and clang-tidy during compilation.

josk uses UTF-8 encoding by default. Check the [UTF-8 Everywhere Manifesto](http://utf8everywhere.org) for details.
