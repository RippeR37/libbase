cmake_minimum_required(VERSION 3.13)

#
# Modules
#

# Core modules
option(LIBBASE_BUILD_MODULE_NET "Build networking module." ON)
option(LIBBASE_BUILD_MODULE_WIN "Build WinApi integration module." ON)
# Optional modules
option(LIBBASE_BUILD_MODULE_WX "Build wxWidgets integration module." OFF)


#
# Dependency resolution & features
#

if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

if(CMAKE_TOOLCHAIN_FILE)
  string(REGEX MATCH "vcpkg\\.cmake$" IS_VCPKG "${CMAKE_TOOLCHAIN_FILE}")
  if(IS_VCPKG)
    if(LIBBASE_BUILD_MODULE_WX)
      list(APPEND VCPKG_MANIFEST_FEATURES "wx")
    endif()
  endif()
endif()


#
# Project setup
#

project(libbase
        VERSION 1.1.0
        LANGUAGES CXX)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")


#
# Build options
#

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(LIBBASE_BUILD_EXAMPLES "Build examples." ON)
  option(LIBBASE_BUILD_TESTS "Build unit tests." ON)
  option(LIBBASE_BUILD_PERFORMANCE_TESTS "Build performance tests." ON)
  option(LIBBASE_CLANG_TIDY "Build with clang-tidy" ON)
else()
  option(LIBBASE_BUILD_EXAMPLES "Build examples." OFF)
  option(LIBBASE_BUILD_TESTS "Build unit tests." OFF)
  option(LIBBASE_BUILDPERFORMANCE_TESTS "Build performance tests." OFF)
  option(LIBBASE_CLANG_TIDY "Build with clang-tidy" OFF)
endif()

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
# Compiler setup
#

include(SetupCompileFlags)

# This sets:
# - LIBBASE_COMPILE_FLAGS,
# - LIBBASE_LINK_FLAGS
# - LIBBASE_OPT_CLANG_TIDY_PROPERTIES - optional C++ property for clang-tidy use
setup_compile_flags()

message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
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

set(LIBBASE_INSTALL_TARGETS libbase)
if(LIBBASE_BUILD_MODULE_NET)
  list(APPEND LIBBASE_INSTALL_TARGETS libbase_net)
endif()
if(LIBBASE_BUILD_MODULE_WIN)
  list(APPEND LIBBASE_INSTALL_TARGETS libbase_win)
endif()
if(LIBBASE_BUILD_MODULE_WX)
  list(APPEND LIBBASE_INSTALL_TARGETS libbase_wx)
endif()

# Install the library
install(TARGETS ${LIBBASE_INSTALL_TARGETS}
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
"find_dependency(glog)\n"
"find_package(curl)\n"
"find_package(wxWidgets)\n"
"find_package(GTest)\n"
"find_package(benchmark)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/${CMAKE_PROJECT_NAME}-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
        DESTINATION share/libbase)
