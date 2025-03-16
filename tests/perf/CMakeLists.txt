if(NOT LIBBASE_BUILD_PERFORMANCE_TESTS)
  message(FATAL_ERROR "Internal libbase CMakeLists.txt error")
endif()


find_package(benchmark CONFIG REQUIRED)


#
# Performance tests target
#

add_executable(libbase_perf_tests "")

target_link_libraries(libbase_perf_tests
  libbase
  benchmark::benchmark)

target_sources(libbase_perf_tests
  PRIVATE
    base/threading/thread_perftests.cc
    main.cc
)
