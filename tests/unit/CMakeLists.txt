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

add_executable(libbase_unittests "")

target_compile_options(libbase_unittests PRIVATE "${LIBBASE_COMPILE_FLAGS}")

target_include_directories(libbase_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/tests/unit/mock/)

target_link_libraries(libbase_unittests PRIVATE
  libbase
  gtest
  gmock)

target_sources(libbase_unittests
  PRIVATE
    base/auto_reset_unittests.cc
    base/barrier_callback_unittests.cc
    base/barrier_closure_unittests.cc
    base/bind_post_task_unittests.cc
    base/bind_unittests.cc
    base/callback_helpers_unittests.cc
    base/memory/weak_ptr_unittests.cc
    base/message_loop/message_loop_impl_unittests.cc
    base/message_loop/message_pump_impl_unittests.cc
    base/sequenced_task_runner_helpers_unittest.cc
    base/sequenced_task_runner_unittests.cc
    base/synchronization/auto_signaller_unittests.cc
    base/synchronization/waitable_event_unittests.cc
    base/task_runner_unittests.cc
    base/threading/thread_unittests.cc
    main.cc
    mock/base/message_loop/mock_message_pump.h
)


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
