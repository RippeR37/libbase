add_subdirectory(simple)

if(LIBBASE_BUILD_MODULE_WX)
  add_subdirectory(wxwidgets_integration)
endif()
