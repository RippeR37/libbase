## `libbase` [![Language](https://img.shields.io/badge/language-C++17-blue.svg)](https://github.com/RippeR37/libbase) [![Documentation](https://img.shields.io/badge/documentation-online-blue.svg)](https://ripper37.github.io/libbase/) [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/ripper37/libbase/master/LICENSE) [![GitHub Releases](https://img.shields.io/github/release/ripper37/libbase.svg)](https://github.com/ripper37/libbase/releases) [![vcpkg release](https://img.shields.io/vcpkg/v/ripper37-libbase)](https://vcpkg.io/en/package/ripper37-libbase)

| Branch | Ubuntu | Windows | MacOS | Documentation | Coverage |
| :----: | :----: | :-----: | :---: | :-----------: | :------: |
| [**`master`**](https://github.com/RippeR37/libbase) | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:master) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:master) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:master) | [![Docs](https://github.com/RippeR37/libbase/actions/workflows/docs.yml/badge.svg?branch=master)](https://ripper37.github.io/libbase/master/) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/master/graph/badge.svg?token=RT0JTLDPJE)](https://app.codecov.io/gh/RippeR37/libbase/branch/master) |
| [`develop`](https://github.com/RippeR37/libbase/tree/develop) | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:develop) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:develop) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:develop) | [![Docs](https://github.com/RippeR37/libbase/actions/workflows/docs.yml/badge.svg?branch=develop)](https://ripper37.github.io/libbase/develop/) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/develop/graph/badge.svg?token=RT0JTLDPJE)](https://app.codecov.io/gh/RippeR37/libbase/branch/develop) |


### Project description

The [`libbase`](https://github.com/RippeR37/libbase/) is a small library that
provides its users with a reimplementation of many useful low-level utilities
from Chromium’s
[`//base`](https://chromium.googlesource.com/chromium/src/base/) module as well
as other useful utilities (e.g. simplified networking stack based on `//net` )
without the need to depend on the whole (or parts of the) Chromium itself.

For more details with examples see the
[documentation](https://ripper37.github.io/libbase/).


### Building `libbase`

#### Dependencies

- [GLOG](https://github.com/google/glog)
- (Optional) [libcurl](https://curl.se/libcurl/) - for networking module
- (Optional) [wxWidgets](https://www.wxwidgets.org/) - for `wxWidgets` integration
- (Optional) [GTest and GMock](https://github.com/google/googletest) - for unit tests
- (Optional) [Google Benchmark](https://github.com/google/benchmark) - for performance tests

Dependencies either have to be already installed and CMake has to be able to
find them with `find_package()` or they can be resolved with `vcpkg`
(recommended).

#### Build with pre-existing `vcpkg` installation

```bash
export VCPKG_ROOT=/path/to/vcpkg/
cmake -S . -B build
cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]
```

or

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]
```

#### Build with internal `vcpkg` installation

If you don't have (or don't want to use) pre-existing `vcpkg` installation you
can ask `libbase` to set up its own internal `vcpkg` installation and use it
with:

```bash
cmake -S . -B build -DLIBBASE_AUTO_VCPKG=1
cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]
```

#### Build manually

You can also manually build and install all required dependencies and then
simply build `libbase` with:

```bash
cmake -S . -B build
cmake --build build [-j <parallel_jobs>] [--config <Release|Debug>]
```

#### Running unit tests

Once you've built `libbase` you can run tests with:

```bash
ctest --test-dir build
```

For more details please refer to the
[building documentation](https://ripper37.github.io/libbase/master/getting_started/building.html).


### Using `libbase` in your project

You can install `libbase` in multiple ways - either by manually building it or
using a package manager like `vcpkg` (recommended) to do it for you. Once you've
installed it, you will need to find and link with it.

#### Install with `vcpkg`

To install `libbase` with `vcpkg` in the
[_manifest mode_](https://learn.microsoft.com/en-us/vcpkg/concepts/manifest-mode#manifest-files-in-projects)
(recommended) simply add `ripper37-libbase` dependency in your `vcpkg.json`
file:

```jsonc
{
  "name": "your-project",
  "dependencies": [
    "ripper37-libbase"
  ]
}
```

Alternatively you can install `libbase` system-wide in the
[_classic mode_](https://learn.microsoft.com/sv-se/vcpkg/concepts/classic-mode)
by executing
[`vcpkg install` command](https://learn.microsoft.com/en-us/vcpkg/commands/install#synopsis)
in your terminal:

```bash
vcpkg install ripper37-libbase
```

#### Import into your CMake project with FetchContent

If you prefer to manually import your dependencies with CMake's `FetchContent`
you can import `libbase` in your project by adding this to your `CMakeFiles.txt`
script:

```cmake
include(FetchContent)
FetchContent_Declare(
    libbase
    GIT_REPOSITORY https://github.com/ripper37/libbase.git
    GIT_TAG        <commit_or_tag_to_fetch>
)
FetchContent_MakeAvailable(libbase)
```

> [!CAUTION]
> This doesn't auto-resolve `libbase` dependencies by itself so you will either
> have to pre-install them manually (so that `find_package()` will find them) or
> declare and make them available earlier in your CMake script with
> `FetchContent` as well.


#### Manual

Lastly you can simply build and install `libbase` manually. Please refer to the
[building section above](https://github.com/RippeR37/libbase?tab=readme-ov-file#building-libbase)
on how to build the library and dependencies. Once built, make sure to install
them all with:

```bash
cmake --install build_directory [--prefix <install_path_prefix>]
```

#### Add to your CMake project

Once you've installed `libbase` in your system, simply ask CMake to find it and
link all with:

```cmake
find_package(libbase CONFIG REQUIRED [COMPONENTS <optional_component>...])
target_link_libraries(your_target PRIVATE libbase::libbase [<optional_target>...])
```

Available optional components:

* `net` (target: `libbase::libbase_net`) - networking module (enabled by default)
* `win` (target: `libbase::libbase_win`) - integration with WinAPI
* `wx` (target: `libbase::libbase_wx`) - integration with wxWidgets

For more details please refer to the
[using documentation](https://ripper37.github.io/libbase/master/getting_started/using.html)
or check out the
[CMake-based example project](https://github.com/RippeR37/libbase-example-cmake).


### Support

#### Requirements

* Compiler with C++17 support
* CMake (>= v3.13)

#### Platforms tested

* Linux
* Windows
* MacOS

#### Compilers tested

* GCC (10 through 14)
* Clang (13 through 18)
* MSVC (2022 19.43)

### License

This project is licensed under the [MIT License](LICENSE).
