function(ADD_GLOG_SUBMODULE GLOG_SUBMODULE_PATH)
  set(BUILD_TESTING OFF CACHE STRING "Override GLOG not to build its unit tests")
  set(BUILD_SHARED_LIBS OFF CACHE STRING "Override GLOG not to build as shared library")
  SET(WITH_GFLAGS OFF CACHE STRING "Override GLOG not to build with GFlags")
  add_subdirectory(${GLOG_SUBMODULE_PATH})
  unset(BUILD_TESTING CACHE)
  unset(BUILD_SHARED_LIBS CACHE)
endfunction()