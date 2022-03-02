## `libbase`

| Branch | Ubuntu | Windows | MacOS | Coverage | Perf tests |
| :----: | :----: | :-----: | :---: | :------: | :--------: |
| **`master`** | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/master/graph/badge.svg?token=RT0JTLDPJE)](https://codecov.io/gh/RippeR37/libbase) | [![perf-tests](https://img.shields.io/badge/charts-master-blue)](https://ripper37.github.io/libbase/perf_tests/master/) |
| `develop` | [![Ubuntu](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/ubuntu.yml) | [![Windows](https://github.com/RippeR37/libbase/actions/workflows/windows.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/windows.yml) | [![MacOS](https://github.com/RippeR37/libbase/actions/workflows/macos.yml/badge.svg?branch=develop)](https://github.com/RippeR37/libbase/actions/workflows/macos.yml) | [![codecov](https://codecov.io/gh/RippeR37/libbase/branch/develop/graph/badge.svg?token=RT0JTLDPJE)](https://codecov.io/gh/RippeR37/libbase) | [![perf-tests](https://img.shields.io/badge/charts-develop-blue)](https://ripper37.github.io/libbase/perf_tests/develop/) |


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
