## `libbase`

| Branch | Ubuntu | Windows | MacOS | Coverage | Documentation |
| :----: | :----: | :-----: | :---: | :------: | :-----------: |
| [**`master`**](https://github.com/RippeR37/libbase) | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:master) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:master) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:master) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/master/graph/badge.svg?token=RT0JTLDPJE)](https://app.codecov.io/gh/RippeR37/libbase/branch/master) | [![Docs](https://github.com/RippeR37/libbase/actions/workflows/docs.yml/badge.svg?branch=master)](https://ripper37.github.io/libbase/master/) |
| [`develop`](https://github.com/RippeR37/libbase/tree/develop) | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml?query=branch:develop) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml?query=branch:develop) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml?query=branch:develop) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/develop/graph/badge.svg?token=RT0JTLDPJE)](https://app.codecov.io/gh/RippeR37/libbase/branch/develop) | [![Docs](https://github.com/RippeR37/libbase/actions/workflows/docs.yml/badge.svg?branch=develop)](https://ripper37.github.io/libbase/develop/) |


### Project description

To-do...

### Building `libbase`

#### Building with CMake

```bash
git clone --recurse-submodules https://github.com/RippeR37/libbase.git
cd libbase
cmake -S . -B build
cmake --build build
```

#### Running unit tests

```bash
ctest --test-dir build
```

#### Requirements

* Compiler with C++17 support
* CMake (>= v3.13)

#### Platforms tested

* Linux
* Windows
* MacOS

#### Compilers tested

* GCC (7 through 10)
* Clang (9 through 12)
* MSVC (2019 19.29)

#### Dependencies

All dependencies are managed as submodules within `third_party/` directory.

- [GLOG](https://github.com/google/glog)
- (Optional) [GTest and GMock](https://github.com/google/googletest)
- (Optional) [Google Benchmark](https://github.com/google/benchmark)

#### License

This project is licensed under the [MIT License](LICENSE).
