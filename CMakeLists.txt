cmake_minimum_required(VERSION 3.13)

#
# Dependency resolution
#

option(LIBBASE_DEPENDENCIES_USE_VCPKG "Fetch dependencies with VCPKG" ON)

if (LIBBASE_DEPENDENCIES_USE_VCPKG)
  if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  else()
    message(WARNING "Requested to use VCPKG for dependency resolution but VCPKG_ROOT environment variable is not set, ignoring")
  endif()
endif()


#
# Project setup
#

project(libbase
        VERSION 1.0.1
        LANGUAGES CXX)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

#
# Build options
#

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(LIBBASE_BUILD_EXAMPLES "Build examples." ON)
  option(LIBBASE_BUILD_TESTS "Build unit tests." ON)
  option(LIBBASE_CODE_COVERAGE "Compute code coverage." OFF)
  option(LIBBASE_BUILD_PERFORMANCE_TESTS "Build performance tests." ON)
  option(LIBBASE_BUILD_DOCS "Build documentation." OFF)
  option(LIBBASE_CLANG_TIDY "Build with clang-tidy" ON)
else()
  option(LIBBASE_BUILD_EXAMPLES "Build examples." OFF)
  option(LIBBASE_BUILD_TESTS "Build unit tests." OFF)
  option(LIBBASE_CODE_COVERAGE "Compute code coverage." OFF)
  option(LIBBASE_BUILDPERFORMANCE_TESTS "Build performance tests." OFF)
  option(LIBBASE_BUILD_DOCS "Build documentation." OFF)
  option(LIBBASE_CLANG_TIDY "Build with clang-tidy" OFF)
endif()


#
# Compiler setup
#

include(SetupCompileFlags)

# This sets:
# - LIBBASE_COMPILE_FLAGS,
# - LIBBASE_LINK_FLAGS
# - LIBBASE_OPT_CLANG_TIDY_PROPERTIES - optional C++ property for clang-tidy use
setup_compile_flags()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler flags: '${LIBBASE_COMPILE_FLAGS}'")
message(STATUS "Linker flags: '${LIBBASE_LINK_FLAGS}'")
message(STATUS "Preprocesor defines: '${LIBBASE_DEFINES}'")
message(STATUS "Clang-tidy property: '${LIBBASE_OPT_CLANG_TIDY_PROPERTIES}'")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


#
# Dependencies
#

# Threads
find_package(Threads REQUIRED)
# GLOG
find_package(glog CONFIG REQUIRED)


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

# Documentation
if (LIBBASE_BUILD_DOCS)
  add_subdirectory(docs)
endif()


#
# Installation
#

# Install the library
install(TARGETS libbase
        EXPORT libbase_targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Install headers
install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/base"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libbase"
        FILES_MATCHING PATTERN "*.h")

# Install license file
install(FILES "${CMAKE_SOURCE_DIR}/LICENSE"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/libbase"
        RENAME "copyright")

# Generate and install *-targets.cmake
install(EXPORT libbase_targets
        FILE libbase-targets.cmake
        NAMESPACE libbase::
        DESTINATION share/libbase)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"find_dependency(glog CONFIG REQUIRED)\n"
"find_dependency(GTest CONFIG QUIET)\n"
"find_dependency(benchmark CONFIG QUIET)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/${CMAKE_PROJECT_NAME}-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
        DESTINATION share/libbase)
