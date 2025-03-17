function(SETUP_COVERAGE_TARGET_GCC)
  cmake_parse_arguments(COVERAGE_ARG
    ""
    "NAME;TEST_TARGET"
    "EXCLUDE_PATHS"
    ${ARGN})

  string(REGEX MATCH "[0-9]+" LLVM_VERSION ${CMAKE_CXX_COMPILER_VERSION})

  message(STATUS "Looking for lcov")
  find_program(LCOV_PATH NAMES lcov lcov-${LLVM_VERSION} lcov-${LLVM_VERSION}.0)
  if(LCOV_PATH)
    message(STATUS "Looking for lcov - found")
  else()
    message(STATUS "Looking for lcov - not found")
  endif()

  message(STATUS "Looking for gcov")
  find_program(GCOV_PATH NAMES gcov gcov-${LLVM_VERSION} gcov-${LLVM_VERSION}.0)
  if(GCOV_PATH)
    message(STATUS "Looking for gcov - found")
  else()
    message(STATUS "Looking for gcov - not found")
  endif()

  message(STATUS "Looking for genhtml")
  find_program(GENHTML_PATH NAMES genhtml)
  if(GENHTML_PATH)
    message(STATUS "Looking for genhtml - found")
  else()
    message(STATUS "Looking for genhtml - not found")
  endif()

  if(NOT LCOV_PATH OR NOT GCOV_PATH OR NOT GENHTML_PATH)
    message(FATAL_ERROR "Couldn't find lcov, gcov or genhtml - code coverage generation failed")
  endif()

  add_custom_target(${COVERAGE_ARG_NAME} ALL
    # Step 0 - cleanup
    COMMAND echo "Step 0"
    COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} -directory . --zerocounters
    COMMAND echo "Step 1"
    COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --ignore-errors mismatch -c -i -d . -o ${COVERAGE_ARG_NAME}.base
    COMMAND echo "Step 2"

    # Step 1 - Run tests
    COMMAND $<TARGET_FILE:${COVERAGE_ARG_TEST_TARGET}> > /dev/null

    # Step 2 - Creating coverage report
    COMMAND echo "Step 3"
    COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --ignore-errors mismatch --directory . --capture --output-file ${COVERAGE_ARG_NAME}.info
    COMMAND echo "Step 4"
    COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --ignore-errors mismatch -a ${COVERAGE_ARG_NAME}.base -a ${COVERAGE_ARG_NAME}.info --output-file ${COVERAGE_ARG_NAME}.total
    COMMAND echo "Step 5"
    COMMAND ${LCOV_PATH} --gcov-tool ${GCOV_PATH} --ignore-errors unused --remove ${COVERAGE_ARG_NAME}.total ${COVERAGE_ARG_EXCLUDE_PATHS} --output-file ${PROJECT_BINARY_DIR}/${COVERAGE_ARG_NAME}.info.cleaned
    COMMAND echo "Step 6"
    COMMAND ${GENHTML_PATH} -o ${COVERAGE_ARG_NAME} ${PROJECT_BINARY_DIR}/${COVERAGE_ARG_NAME}.info.cleaned
    COMMAND echo "Step 7"
    COMMAND ${CMAKE_COMMAND} -E remove ${COVERAGE_ARG_NAME}.base ${COVERAGE_ARG_NAME}.total ${PROJECT_BINARY_DIR}/${COVERAGE_ARG_NAME}.info.cleaned
    COMMAND echo "Step 7"

    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    DEPENDS ${COVERAGE_ARG_TEST_TARGET}
    COMMENT "Generating 'gcov'-based code coverage report"
  )

endfunction()

function(SETUP_COVERAGE_TARGET_CLANG)
  cmake_parse_arguments(COVERAGE_ARG
    ""
    "NAME;TEST_TARGET"
    "EXCLUDE_PATHS"
    ${ARGN})

  string(REGEX MATCH "[0-9]+" LLVM_VERSION ${CMAKE_CXX_COMPILER_VERSION})

  message(STATUS "Looking for llvm-profdata")
  find_program(LLVM_PROFDATA_PATH NAMES llvm-profdata-${LLVM_VERSION} llvm-profdata-${LLVM_VERSION}.0)
  if(LLVM_PROFDATA_PATH)
    message(STATUS "Looking for llvm-profdata - found")
  else()
    message(STATUS "Looking for llvm-profdata - not found")
  endif()

  message(STATUS "Looking for llvm-cov")
  find_program(LLVM_COV_PATH NAMES llvm-cov-${LLVM_VERSION} llvm-cov-${LLVM_VERSION}.0)
  if(LLVM_COV_PATH)
    message(STATUS "Looking for llvm-cov - found")
  else()
    message(STATUS "Looking for llvm-cov - not found")
  endif()

  if(NOT LLVM_PROFDATA_PATH OR NOT LLVM_COV_PATH)
    message(FATAL_ERROR "Couldn't find llvm-profdata or llvm-cov - code coverage generation failed")
  endif()

  add_custom_target(${COVERAGE_ARG_NAME} ALL
    # Step 0 - Cleanup
    COMMAND rm -r -f ${PROJECT_BINARY_DIR}/coverage
    COMMAND rm -f default.profraw
    COMMAND rm -f default.profdata

    # Step 1 - Run tests
    COMMAND ./$<TARGET_FILE_NAME:${COVERAGE_ARG_TEST_TARGET}> > /dev/null

    # Step 2 - Index raw profiles
    COMMAND ${LLVM_PROFDATA_PATH} merge -sparse default.profraw -o default.profdata

    # Step 3 - Create coverage report
    COMMAND
      ${LLVM_COV_PATH} show ./$<TARGET_FILE_NAME:${COVERAGE_ARG_TEST_TARGET}>
      -instr-profile=default.profdata -format=html
      -output-dir="${PROJECT_BINARY_DIR}/coverage/" -Xdemangler=c++filt
      -ignore-filename-regex="${COVERAGE_ARG_EXCLUDE_PATHS}"
      --show-branches=percent

    # Step 4 - Print summary to console
    COMMAND
      ${LLVM_COV_PATH} report ./$<TARGET_FILE_NAME:${COVERAGE_ARG_TEST_TARGET}>
      -instr-profile=default.profdata
      -ignore-filename-regex="${COVERAGE_ARG_EXCLUDE_PATHS}"

    # Step 5 - Print path to HTML report
    COMMAND echo ""
    COMMAND echo "Full HTML code coverage report can be found in: ${PROJECT_BINARY_DIR}/coverage/"

    WORKING_DIRECTORY $<TARGET_FILE_DIR:${COVERAGE_ARG_TEST_TARGET}>
    DEPENDS ${COVERAGE_ARG_TEST_TARGET}
    COMMENT "Generating 'llvm-cov'-based code coverage report"
  )
endfunction()
