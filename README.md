# JSON of Skyrim (josk)

[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-brightgreen.svg)](https://opensource.org/license/gpl-3-0)

Generate JSON data from a Skyrim modlist.

On Windows platforms, Windows 10 Version 1903 (May 2019 Update) or newer is required.

## Contributing

josk encourages community involvement and contributions, check [CONTRIBUTING.md](CONTRIBUTING.md) for details. Contributors can be found in the [contributors list](https://github.com/joseasoler/json_of_skyrim/graphs/contributors).

## License

josk is licensed under the GNU General Public License version 3. See the [LICENSE](LICENSE) file for details.

## Building

Building requires [CMake](https://cmake.org) and a [compiler with C++23 support](https://en.cppreference.com/w/cpp/compiler_support#cpp23).

josk is cross-platform, but it is mostly tested on Windows with MSVC. Contributions to improve support are welcome!

### CMake options

* `CMAKE_COMPILE_WARNING_AS_ERROR`: Compilers treat warnings as errors. Off by default.
* `JOSK_CLANG_TIDY`: Analyze the project using [clang-tidy](https://clang.llvm.org/extra/clang-tidy). Warnings will be treated as errors if `CMAKE_COMPILE_WARNING_AS_ERROR` is enabled. Off by default.

### Dependencies

The following dependencies must be available through the `find_package` CMake feature.

* **[CLI11](https://github.com/CLIUtils/CLI11)**: Command line parser for C++11.

### vcpkg support

Dependencies can optionally be retrieved and built using [vcpkg](https://github.com/microsoft/vcpkg). This is disabled by default, but it is enabled in the provided CMake presets.

### CMake presets

josk includes a set of CMake presets. More detail about them can be found in the [CONTRIBUTING.md](CONTRIBUTING.md) guide.

## Acknowledgements

Check the [ACKNOWLEDGEMENTS.md](ACKNOWLEDGEMENTS.md) file for details.

