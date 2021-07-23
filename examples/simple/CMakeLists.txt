add_executable(simple ${libbase_SOURCE_DIR}/examples/simple/main.cc)

target_compile_options(simple PRIVATE ${LIBBASE_FLAGS_PROGRAM})
set_target_properties(simple PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(simple PRIVATE libbase)
