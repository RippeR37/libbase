#
# CMake setup
#

cmake_minimum_required(VERSION 2.8)


#
# Project setup
#

set(PROJECT_NAME libbase)
project(${PROJECT_NAME} CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)


#
# Build options
#

option(BUILD_TESTS "Build all tests." ON)


#
# Compiler setup
#

set(CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(WARNINGS "-Wall -Wextra -Wpedantic -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(WARNINGS "/W4 /WX /EHsc")
endif()

if(BUILD_TESTS AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   set(COVERAGE_FLAGS "-g -fprofile-arcs -ftest-coverage -fno-inline -fno-inline-small-functions -fno-default-inline -fno-elide-constructors")
endif()

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${WARNINGS} ${EXTRA_FLAGS} ${COVERAGE_FLAGS}"
    CACHE STRING "Flags used by the compiler during all build types." FORCE)

    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
    message(STATUS "Compiler flags: '${CMAKE_CXX_FLAGS}'")
endif()


#
# Library files
#

file(GLOB_RECURSE LIBBASE_HEADERS ${PROJECT_SOURCE_DIR}/src/base/*.cc)
file(GLOB_RECURSE LIBBASE_SOURCES ${PROJECT_SOURCE_DIR}/src/base/*.h)


#
# Compile library
#

#add_library(libbase ${LIBBASE_HEADERS} ${LIBBASE_SOURCES})
#set_target_properties(libbase PROPERTIES LINKER_LANGUAGE CXX)


#
# Setup executable
#

add_executable(libbase_exe ${PROJECT_SOURCE_DIR}/src/main.cc ${LIBBASE_HEADERS} ${LIBBASE_SOURCES})
#target_link_libraries(libbase_exe libbase)


#
# Setup testing
#

if(BUILD_TESTS)
    add_subdirectory(third_party/googletest)
    enable_testing()
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    include_directories(${PROJECT_SOURCE_DIR}/src/)

    file(GLOB_RECURSE TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/tests/*.cc)

    add_executable(unit_tests ${TEST_SRC_FILES} ${LIBBASE_HEADERS} ${LIBBASE_SOURCES})
    target_link_libraries(unit_tests gtest gtest_main)

    #target_link_libraries(unit_tests libbase)

    add_test(UnitTests unit_tests)

    if(CMAKE_COMPILER_IS_GNUCXX)
        include(CodeCoverage)
        APPEND_COVERAGE_COMPILER_FLAGS()
        set(COVERAGE_LCOV_EXCLUDES '/usr/*' '*/third_party/googletest/*')
        SETUP_TARGET_FOR_COVERAGE_LCOV(
                NAME coverage
                EXECUTABLE ctest -j ${n_cores}
                DEPENDENCIES
                    unit_tests)
    endif()

endif()

