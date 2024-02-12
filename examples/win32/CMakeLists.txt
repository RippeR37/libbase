add_executable(win32 WIN32 "")

target_compile_options(win32 PRIVATE ${LIBBASE_COMPILE_FLAGS})
target_link_libraries(win32 PRIVATE libbase)

target_sources(win32
  PRIVATE
    main.cc
)
