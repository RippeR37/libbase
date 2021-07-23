if(LIBBASE_BUILD_TESTS)
  add_subdirectory(unit)
endif()

if(LIBBASE_BUILD_PERFORMANCE_TESTS)
  add_subdirectory(perf)
endif()
