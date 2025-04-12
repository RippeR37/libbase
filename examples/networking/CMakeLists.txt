add_executable(networking "")

target_compile_options(networking
  PRIVATE
    ${LIBBASE_COMPILE_FLAGS}
)

target_link_libraries(networking
  PRIVATE
    libbase
    libbase_net
)

target_sources(networking
  PRIVATE
    main.cc
)
