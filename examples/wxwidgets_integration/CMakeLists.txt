find_package(wxWidgets CONFIG REQUIRED)

add_executable(wxwidgets_integration_example WIN32 "")

target_compile_options(wxwidgets_integration_example
  PRIVATE
    ${LIBBASE_COMPILE_FLAGS}
)

target_link_libraries(wxwidgets_integration_example
  PRIVATE
    libbase
    libbase_wx
    wxWidgets::wxWidgets
)

target_sources(wxwidgets_integration_example
  PRIVATE
    app.cc
    app.h
    main_frame.cc
    main_frame.h
    main.cc
)
