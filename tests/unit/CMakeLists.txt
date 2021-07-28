if(NOT LIBBASE_BUILD_TESTS)
  message(FATAL_ERROR "Internal libbase CMakeLists.txt error")
endif()


#
# Google Test setup
#

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest build_gtest)


#
# Unittests target
#

file(GLOB_RECURSE LIBBASE_UNITTESTS_SRC CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/tests/unit/main.cc
  ${PROJECT_SOURCE_DIR}/tests/unit/*.h
  ${PROJECT_SOURCE_DIR}/tests/unit/*.cc
  ${PROJECT_SOURCE_DIR}/tests/unit/*.h
  ${PROJECT_SOURCE_DIR}/tests/unit/*.cc)

add_executable(libbase_unittests ${LIBBASE_UNITTESTS_SRC})

target_compile_options(libbase_unittests PRIVATE "${LIBBASE_COMPILE_FLAGS}")

target_include_directories(libbase_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/tests/unit/mock/)

target_link_libraries(libbase_unittests PRIVATE
  libbase
  gtest
  gmock)

include(GoogleTest)
gtest_discover_tests(libbase_unittests)


#
# Code coverage
#

if(LIBBASE_CODE_COVERAGE)
  include(SetupCodeCoverage)

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    setup_coverage_target_gcc(
      NAME coverage
      TEST_TARGET libbase_unittests
      EXCLUDE_PATHS '/usr/*' '*/third_party/*' '*/main.cc' '*/tests/*')
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    setup_coverage_target_clang(
      NAME coverage
      TEST_TARGET libbase_unittests
      EXCLUDE_PATHS "third_party|tests")
  else()
    message(FATAL_ERROR "Code coverage supported only with GCC and Clang compilers")
  endif()
endif()
