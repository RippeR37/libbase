function(SETUP_COMPILE_FLAGS)
  set(DEFINES "")

  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    string(APPEND DEFINES "LIBBASE_IS_LINUX")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    string(APPEND DEFINES "LIBBASE_IS_WINDOWS")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    string(APPEND DEFINES "LIBBASE_IS_MACOS")
  endif()

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      # -Wold-style-cast
      set(WARNINGS "-Wall;-Wextra;-Wpedantic;-Werror;-Wunreachable-code")

      if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(WARNINGS "${WARNINGS};-Wshadow")
      else()
        set(WARNINGS "${WARNINGS};-Wshadow=local")
      endif()
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      set(WARNINGS "/W4;/WX;/EHsc;/permissive-")
  endif()

  if(LIBBASE_BUILD_TESTS AND LIBBASE_CODE_COVERAGE)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      set(COVERAGE_COMPILE_FLAGS ";-g;-fno-inline;-fno-elide-constructors;-fno-inline-small-functions;-fno-default-inline;-fprofile-arcs;-ftest-coverage")
      set(COVERAGE_LINK_FLAGS "-lgcov")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(COVERAGE_COMPILE_FLAGS ";-fprofile-instr-generate;-fcoverage-mapping")
      set(COVERAGE_LINK_FLAGS "-fprofile-instr-generate;-fcoverage-mapping")
    else()
      message(FATAL_ERROR "Code coverage supported only with GCC and Clang compilers")
    endif()
  endif()

  if(LIBBASE_CLANG_TIDY AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CLANG_TIDY_PROPERTIES "CXX_CLANG_TIDY;clang-tidy-10")
  endif()

  if(NOT CONFIGURED_ONCE)
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
      LIBBASE_DEFINES
      "${DEFINES}"
      CACHE STRING "Preprocessor defines"
      FORCE
    )
    set(
      LIBBASE_OPT_CLANG_TIDY_PROPERTIES
      "${CLANG_TIDY_PROPERTIES}"
      CACHE STRING "Properties used to enable clang-tidy when building targets"
      FORCE)
  endif()
endfunction()
