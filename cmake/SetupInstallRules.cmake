
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)


function(libbase_install_target target)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "Target ${target} not found. Please check the target name.")
  endif()

  if(PROJECT_IS_TOP_LEVEL)
    string(REPLACE "_" "-" target_dash "${target}")

    install(TARGETS ${target}
      EXPORT ${target}_targets
      ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(EXPORT ${target}_targets
      FILE ${target_dash}-targets.cmake
      NAMESPACE libbase::
      DESTINATION share/${CMAKE_PROJECT_NAME})
  endif()
endfunction()


function(libbase_install_rules)
  # Install headers
  install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/base"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}"
          FILES_MATCHING PATTERN "*.h")

  # Install license file
  install(FILES "${CMAKE_SOURCE_DIR}/LICENSE"
          DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}"
          RENAME "copyright")

  # Configure project config file and version file
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${CMAKE_PROJECT_NAME}-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/share/${CMAKE_PROJECT_NAME})
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
  )

  # Install the generated config file
  install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-config-version.cmake"
    DESTINATION share/${CMAKE_PROJECT_NAME})
endfunction()
