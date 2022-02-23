add_executable(simple "")

target_compile_options(simple PRIVATE ${LIBBASE_COMPILE_FLAGS})
set_target_properties(simple PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(simple PRIVATE libbase)

target_sources(simple
  PRIVATE
    main.cc
)
