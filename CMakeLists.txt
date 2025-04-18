cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(SetupDependencyResolution)
include(EnsureNotInSourceBuild)

#
# Modules
#

# Core modules
option(LIBBASE_BUILD_MODULE_NET "Build networking module." ON)
option(LIBBASE_BUILD_MODULE_WIN "Build WinApi integration module." ON)
# Optional modules
option(LIBBASE_BUILD_MODULE_WX "Build wxWidgets integration module." OFF)

# Dependency resolution for optional modules/features
if(LIBBASE_BUILD_USES_VCPKG)
  if(LIBBASE_BUILD_MODULE_WX)
    list(APPEND VCPKG_MANIFEST_FEATURES "wx")
  endif()
endif()


#
# Project setup
#

project(libbase
        VERSION 1.1.0
        LANGUAGES CXX)

include(ProjectIsTopLevel)


#
# Build options
#

option(LIBBASE_BUILD_EXAMPLES "Build examples." ${PROJECT_IS_TOP_LEVEL_OPT})
option(LIBBASE_BUILD_TESTS "Build unit tests." ${PROJECT_IS_TOP_LEVEL_OPT})
option(LIBBASE_BUILD_PERFORMANCE_TESTS "Build performance tests." ${PROJECT_IS_TOP_LEVEL_OPT})
option(LIBBASE_CLANG_TIDY "Build with clang-tidy" ${PROJECT_IS_TOP_LEVEL_OPT})

option(LIBBASE_CODE_COVERAGE "Compute code coverage." OFF)
option(LIBBASE_BUILD_DOCS "Build documentation." OFF)
option(LIBBASE_BUILD_ASAN "Build with Address Sanitizer enabled" OFF)
option(LIBBASE_BUILD_TSAN "Build with Thread Sanitizer enabled" OFF)

if(LIBBASE_BUILD_MODULE_WIN AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(LIBBASE_BUILD_MODULE_WIN OFF CACHE BOOL "" FORCE)
endif()

set(LIBBASE_OUTPUT_NAME "libbase" CACHE STRING
  "The library's output basename. Modify to resolve name clashes.")


#
# Targets
#

include(SetupCompileFlags)
include(SetupInstallRules)

# libbase library
add_subdirectory(src)

if(PROJECT_IS_TOP_LEVEL)
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

  # Documentation
  if (LIBBASE_BUILD_DOCS)
    add_subdirectory(docs)
  endif()
endif()


#
# Installation
#

if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_SKIP_INSTALL_RULES)
  libbase_install_rules()
endif()
