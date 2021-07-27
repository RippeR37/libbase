function (SETUP_COMPILE_FLAGS)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      # -Wold-style-cast
      set(WARNINGS "-Wall;-Wextra;-Wpedantic;-Werror;-Wunreachable-code")

      if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(WARNINGS "${WARNINGS};-Wshadow")
      else()
        set(WARNINGS "${WARNINGS};-Wshadow=local")
      endif()
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      set(WARNINGS "/W4;/WX;/EHsc;/permissive-")
  endif()

  if(LIBBASE_BUILD_TESTS AND LIBBASE_CODE_COVERAGE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(COVERAGE_COMPILE_FLAGS ";-g;-fprofile-arcs;-ftest-coverage;-fno-inline;-fno-inline-small-functions;-fno-default-inline;-fno-elide-constructors")
    set(COVERAGE_LINK_FLAGS "-lgcov")
  endif()

  if(LIBBASE_CLANG_TIDY AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CLANG_TIDY_PROPERTIES "CXX_CLANG_TIDY;clang-tidy-10")
  endif()

  if (NOT CONFIGURED_ONCE)
    set(
      LIBBASE_COMPILE_FLAGS
      "${WARNINGS}${COVERAGE_COMPILE_FLAGS}"
      CACHE STRING "Flags used by the compiler to build targets"
      FORCE)
    set(
      LIBBASE_LINK_FLAGS
      "${COVERAGE_LINK_FLAGS}"
      CACHE STRING "Flags used by the linker to link targets"
      FORCE)
    set(
      LIBBASE_OPT_CLANG_TIDY_PROPERTIES
      "${CLANG_TIDY_PROPERTIES}"
      CACHE STRING "Properties used to enable clang-tidy when building targets"
      FORCE)
  endif()
endfunction()
