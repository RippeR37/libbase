#
# CMake setup
#

cmake_minimum_required(VERSION 2.8)


#
# Project setup
#

set(PROJECT_NAME libbase)
project(${PROJECT_NAME} CXX)


#
# Compiler setup
#

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Werror")

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

add_subdirectory(third_party/googletest)
enable_testing()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/)

file(GLOB_RECURSE TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/tests/*.cc)

add_executable(unit_tests ${TEST_SRC_FILES} ${LIBBASE_HEADERS} ${LIBBASE_SOURCES})
target_link_libraries(unit_tests gtest gtest_main)

target_compile_options(unit_tests PUBLIC --coverage)
target_link_libraries(unit_tests D:/Programy/LLVM/lib/clang/8.0.1/lib/windows/clang_rt.profile-x86_64.lib)

#target_link_libraries(unit_tests libbase)

add_test(UnitTests unit_tests)
