add_subdirectory(simple)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_subdirectory(win32)
endif ()
