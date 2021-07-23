if(NOT LIBBASE_BUILD_PERFORMANCE_TESTS)
  message(FATAL_ERROR "Internal libbase CMakeLists.txt error")
endif()


#
# Google Benchmark setup
#

SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
SET(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "")
SET(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
SET(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE BOOL "")
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googlebenchmark build_gbench)


#
# Performance tests target
#

file(GLOB_RECURSE LIBBASE_PERF_TEST_SRC CONFIGURE_DEPENDS 
  ${PROJECT_SOURCE_DIR}/tests/perf/*.h
  ${PROJECT_SOURCE_DIR}/tests/perf/*.cc)

add_executable(libbase_perf_tests ${LIBBASE_PERF_TEST_SRC})

target_link_libraries(libbase_perf_tests
  libbase
  benchmark::benchmark
  benchmark::benchmark_main)
