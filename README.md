## `libbase` [![Language](https://img.shields.io/badge/language-C++17-blue.svg)](https://github.com/RippeR37/libbase) [![Documentation](https://img.shields.io/badge/documentation-online-blue.svg)](https://ripper37.github.io/libbase/) [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/ripper37/libbase/master/LICENSE) [![GitHub Releases](https://img.shields.io/github/release/ripper37/libbase.svg)](https://github.com/ripper37/libbase/releases)

| Branch | Ubuntu | Windows | MacOS | Documentation | Coverage |
| :----: | :----: | :-----: | :---: | :-----------: | :------: |
| [**`master`**](https://github.com/RippeR37/libbase) | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:master) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:master) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:master) | [![Docs](https://github.com/RippeR37/libbase/actions/workflows/docs.yml/badge.svg?branch=master)](https://ripper37.github.io/libbase/master/) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/master/graph/badge.svg?token=RT0JTLDPJE)](https://app.codecov.io/gh/RippeR37/libbase/branch/master) |
| [`develop`](https://github.com/RippeR37/libbase/tree/develop) | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:develop) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:develop) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:develop) | [![Docs](https://github.com/RippeR37/libbase/actions/workflows/docs.yml/badge.svg?branch=develop)](https://ripper37.github.io/libbase/develop/) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/develop/graph/badge.svg?token=RT0JTLDPJE)](https://app.codecov.io/gh/RippeR37/libbase/branch/develop) |


### Project description

The [`libbase`](https://github.com/RippeR37/libbase/) is a small library that
provides its users with a reimplementation of many useful low-level utilities
known from Chromium’s
[`//base`](https://chromium.googlesource.com/chromium/src/base/) module without
the need to depend on the whole (or parts of the) Chromium itself.

For more details with examples see the
[documentation](https://ripper37.github.io/libbase/).


### Building `libbase`

#### Building with CMake

```bash
git clone https://github.com/RippeR37/libbase.git
cd libbase
cmake -S . -B build
cmake --build build
```

#### Running unit tests

```bash
ctest --test-dir build
```

#### Using `libbase` in your project

For an example of how to use `libbase` in your project refer to the
documentation or check out the
[CMake-based example project](https://github.com/RippeR37/libbase-example-cmake).

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

#### Dependencies

- [GLOG](https://github.com/google/glog)
- (Optional) [GTest and GMock](https://github.com/google/googletest)
- (Optional) [Google Benchmark](https://github.com/google/benchmark)

Dependencies either have to be already installed and CMake has to be able to
find them with `find_package()` or they can be resolved with VCPKG.

If you wish for CMake to use VCPKG to resolve dependencies you have to set
`VCPKG_ROOT` environment variable before configuring the project.

#### License

This project is licensed under the [MIT License](LICENSE).
