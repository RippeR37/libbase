if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
    FATAL_ERROR
    "In-source builds are not supported. \n"
    "Please read the README.md or project's documentation to see how to build "
    "this project.\n"
    "You may have to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()
