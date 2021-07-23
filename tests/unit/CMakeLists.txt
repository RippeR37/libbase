if(NOT LIBBASE_BUILD_TESTS)
  message(FATAL_ERROR "Internal libbase CMakeLists.txt error")
endif()


#
# Google Test setup
#

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest build_gtest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


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

target_compile_options(libbase_unittests PRIVATE "${LIBBASE_FLAGS_TESTS}")

target_include_directories(libbase_unittests PRIVATE 
  ${PROJECT_SOURCE_DIR}/tests/unit/mock/)

target_link_libraries(libbase_unittests PRIVATE
  libbase
  gtest
  gmock)

add_test(libbase_unittests libbase_unittests)


#
# Code coverage
#

if(CODE_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
  include(CodeCoverage)
  APPEND_COVERAGE_COMPILER_FLAGS()
  set(COVERAGE_LCOV_EXCLUDES '/usr/*' '*/third_party/*' '*/main.cc' '*/tests/*')
  SETUP_TARGET_FOR_COVERAGE_LCOV(
    NAME coverage
    EXECUTABLE ctest -j ${n_cores}
    DEPENDENCIES libbase_unittests)
endif()
