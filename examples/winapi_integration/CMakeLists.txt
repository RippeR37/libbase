add_executable(winapi_integration_example
  WIN32
  ""
)

target_compile_options(winapi_integration_example
  PRIVATE
    ${LIBBASE_COMPILE_FLAGS}
)

target_link_libraries(winapi_integration_example
  PRIVATE
    libbase
    libbase_win
)

target_sources(winapi_integration_example
  PRIVATE
    main.cc
)
