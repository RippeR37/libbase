file(GLOB_RECURSE LIBBASE_SRC CONFIGURE_DEPENDS
  ${libbase_SOURCE_DIR}/src/base/*.h
  ${libbase_SOURCE_DIR}/src/base/*.cc)

add_library(libbase STATIC ${LIBBASE_SRC})

target_compile_features(libbase PUBLIC cxx_std_17)
target_compile_options(libbase PRIVATE ${LIBBASE_COMPILE_FLAGS})
target_compile_definitions(libbase PUBLIC ${LIBBASE_DEFINES})
set_target_properties(libbase PROPERTIES
  CXX_EXTENSIONS OFF
  ${LIBBASE_OPT_CLANG_TIDY_PROPERTIES})

target_include_directories(libbase PUBLIC ${libbase_SOURCE_DIR}/src/)
target_link_libraries(libbase PUBLIC
  ${LIBBASE_LINK_FLAGS}
  Threads::Threads
  glog::glog)
