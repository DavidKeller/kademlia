# SPDX-License-Identifier: MIT

include(CMakePackageConfigHelpers)

export(
  TARGETS kademlia
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
)

write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
)

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/BuildTreeProjectConfig.cmake.in"
  ${PROJECT_NAME}Config.cmake
  @ONLY
)

export(PACKAGE ${PROJECT_NAME})

if(KADEMLIA_INSTALL)
  install(
    TARGETS kademlia
    EXPORT ${PROJECT_NAME}Targets
    FILE_SET HEADERS
  )

  install(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION share/${PROJECT_NAME}
  )

  configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/InstalledProjectConfig.cmake.in"
    installed_config/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION share/${PROJECT_NAME}
  )

  install(
    FILES
      "${PROJECT_BINARY_DIR}/installed_config/${PROJECT_NAME}Config.cmake"
      "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION share/${PROJECT_NAME}
  )
endif()
