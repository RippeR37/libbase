add_subdirectory(simple)

if(LIBBASE_BUILD_MODULE_NET)
  add_subdirectory(networking)
endif()

if(LIBBASE_BUILD_MODULE_WIN)
  add_subdirectory(winapi_integration)
endif()

if(LIBBASE_BUILD_MODULE_WX)
  add_subdirectory(wxwidgets_integration)
endif()
