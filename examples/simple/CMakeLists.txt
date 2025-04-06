add_executable(simple "")

target_compile_options(simple PRIVATE ${LIBBASE_COMPILE_FLAGS})

if(LIBBASE_BUILD_MODULE_NET)
  target_link_libraries(simple PRIVATE libbase libbase_net)
else()
  target_link_libraries(simple PRIVATE libbase)
endif()

target_sources(simple
  PRIVATE
    main.cc
)
