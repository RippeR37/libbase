cmake_minimum_required(VERSION 3.11)

#
# Project setup
#

project(libbase
        VERSION 0.1
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)


#
# Build options
#

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(LIBBASE_BUILD_EXAMPLES "Build examples." ON)
  option(LIBBASE_BUILD_TESTS "Build unit tests." ON)
  option(LIBBASE_CODE_COVERAGE "Compute code coverage." OFF)
  option(LIBBASE_BUILD_PERFORMANCE_TESTS "Build performance tests." ON)
else()
  option(LIBBASE_BUILD_EXAMPLES "Build examples." OFF)
  option(LIBBASE_BUILD_TESTS "Build unit tests." OFF)
  option(LIBBASE_CODE_COVERAGE "Compute code coverage." OFF)
  option(LIBBASE_BUILDPERFORMANCE_TESTS "Build performance tests." OFF)
endif()


#
# Compiler setup
#

include(SetupCompileFlags)
setup_compile_flags() # sets LIBBASE_COMPILE_FLAGS and LIBBASE_LINK_FLAGS

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler flags: '${LIBBASE_COMPILE_FLAGS}'")
message(STATUS "Linker flags: '${LIBBASE_LINK_FLAGS}'")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


#
# Dependencies
#

# Threads
find_package(Threads REQUIRED)

# GLOG
include(AddGlogSubmodule)
add_glog_submodule(third_party/glog)


#
# Targets
#

# libbase library
add_subdirectory(src)

# Example applications
if (LIBBASE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Tests
if(LIBBASE_BUILD_TESTS)
  enable_testing()
endif()
if(LIBBASE_BUILD_TESTS OR LIBBASE_BUILD_PERFORMANCE_TESTS)
  add_subdirectory(tests)
endif()
