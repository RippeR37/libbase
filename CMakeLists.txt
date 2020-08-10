#
# CMake setup
#

cmake_minimum_required(VERSION 2.8)


#
# Project setup
#

set(PROJECT_NAME libbase)
project(${PROJECT_NAME} CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

#
# Build options
#

option(BUILD_TESTS "Build all tests." ON)


#
# Compiler setup
#

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Some warnings are enabled per-target because third-party code might not
    # build with them.
    set(WARNINGS "-Wall -Wextra -Wpedantic -Werror -Wunreachable-code")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(WARNINGS "${WARNINGS} -Wshadow")
    else()
      set(WARNINGS "${WARNINGS} -Wshadow=local")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(WARNINGS "/W4 /WX /EHsc")
endif()

if(BUILD_TESTS AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   set(COVERAGE_FLAGS "-g -fprofile-arcs -ftest-coverage -fno-inline -fno-inline-small-functions -fno-default-inline -fno-elide-constructors")
endif()

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${WARNINGS} ${EXTRA_FLAGS} ${COVERAGE_FLAGS}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)

    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
    message(STATUS "Compiler flags: '${CMAKE_CXX_FLAGS}'")
endif()


#
# Library files
#

file(GLOB_RECURSE LIBBASE_HEADERS ${PROJECT_SOURCE_DIR}/src/base/*.h)
file(GLOB_RECURSE LIBBASE_SOURCES ${PROJECT_SOURCE_DIR}/src/base/*.cc)


#
# Compile library
#

#add_library(libbase ${LIBBASE_HEADERS} ${LIBBASE_SOURCES})
#set_target_properties(libbase PROPERTIES LINKER_LANGUAGE CXX)


#
# Setup executable
#

add_executable(libbase_exe ${PROJECT_SOURCE_DIR}/src/main.cc ${LIBBASE_HEADERS} ${LIBBASE_SOURCES})
target_link_libraries(libbase_exe Threads::Threads)
#target_link_libraries(libbase_exe libbase)

target_compile_options(libbase_exe PRIVATE "-Wold-style-cast")


#
# Setup testing
#

if(BUILD_TESTS)

    #
    # Unit-testing
    #

    add_subdirectory(third_party/googletest)
    enable_testing()
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
    include_directories(${PROJECT_SOURCE_DIR}/src/)
    include_directories(${PROJECT_SOURCE_DIR}/tests/mock/)

    file(GLOB_RECURSE LIBBASE_TEST_HEADERS ${PROJECT_SOURCE_DIR}/tests/src/*.h)
    file(GLOB_RECURSE LIBBASE_TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/src/*.cc)
    file(GLOB_RECURSE LIBBASE_MOCK_HEADERS ${PROJECT_SOURCE_DIR}/tests/mock/*.h)
    file(GLOB_RECURSE LIBBASE_MOCK_SOURCES ${PROJECT_SOURCE_DIR}/tests/mock/*.cc)

    add_executable(unit_tests
      ${LIBBASE_TEST_HEADERS}
      ${LIBBASE_TEST_SOURCES}
      ${LIBBASE_MOCK_HEADERS}
      ${LIBBASE_MOCK_SOURCES}
      ${LIBBASE_HEADERS}
      ${LIBBASE_SOURCES})
    target_link_libraries(unit_tests gtest gtest_main gmock gmock_main Threads::Threads)
    #target_link_libraries(unit_tests libbase)

    target_compile_options(libbase_exe PRIVATE "-Wold-style-cast")

    add_test(UnitTests unit_tests)

    if(CMAKE_COMPILER_IS_GNUCXX)
        include(CodeCoverage)
        APPEND_COVERAGE_COMPILER_FLAGS()
        set(COVERAGE_LCOV_EXCLUDES '/usr/*' '*/third_party/*' '*/main.cc' '*/tests/*')
        SETUP_TARGET_FOR_COVERAGE_LCOV(
                NAME coverage
                EXECUTABLE ctest -j ${n_cores}
                DEPENDENCIES
                    unit_tests)
    endif()

    #
    # Performance testing
    #

    # Google Benchmark setup
    SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
    SET(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "")
    SET(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
    SET(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE BOOL "")
    add_subdirectory(third_party/googlebenchmark)

    # Performance tests
    include_directories(${benchmark_SOURCE_DIR}/include ${benchmark_SOURCE_DIR})
    file(GLOB_RECURSE LIBBASE_PERF_TEST_HEADERS ${PROJECT_SOURCE_DIR}/tests/perf/*.h)
    file(GLOB_RECURSE LIBBASE_PERF_TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/perf/*.cc)
    add_executable(perf_tests
      ${LIBBASE_PERF_TEST_HEADERS}
      ${LIBBASE_PERF_TEST_SOURCES}
      ${LIBBASE_HEADERS}
      ${LIBBASE_SOURCES})
    target_link_libraries(perf_tests
      benchmark::benchmark
      benchmark::benchmark_main
      Threads::Threads)

endif()
