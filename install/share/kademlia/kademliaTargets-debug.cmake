#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "kademlia::kademlia" for configuration "Debug"
set_property(TARGET kademlia::kademlia APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(kademlia::kademlia PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/kademlia.lib"
  )

list(APPEND _cmake_import_check_targets kademlia::kademlia )
list(APPEND _cmake_import_check_files_for_kademlia::kademlia "${_IMPORT_PREFIX}/lib/kademlia.lib" )

# Import target "kademlia::kademlia-error" for configuration "Debug"
set_property(TARGET kademlia::kademlia-error APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(kademlia::kademlia-error PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/kademlia-error.lib"
  )

list(APPEND _cmake_import_check_targets kademlia::kademlia-error )
list(APPEND _cmake_import_check_files_for_kademlia::kademlia-error "${_IMPORT_PREFIX}/lib/kademlia-error.lib" )

# Import target "kademlia::kademlia-endpoint" for configuration "Debug"
set_property(TARGET kademlia::kademlia-endpoint APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(kademlia::kademlia-endpoint PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/kademlia-endpoint.lib"
  )

list(APPEND _cmake_import_check_targets kademlia::kademlia-endpoint )
list(APPEND _cmake_import_check_files_for_kademlia::kademlia-endpoint "${_IMPORT_PREFIX}/lib/kademlia-endpoint.lib" )

# Import target "kademlia::kademlia-impl" for configuration "Debug"
set_property(TARGET kademlia::kademlia-impl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(kademlia::kademlia-impl PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/kademlia-impl.lib"
  )

list(APPEND _cmake_import_check_targets kademlia::kademlia-impl )
list(APPEND _cmake_import_check_files_for_kademlia::kademlia-impl "${_IMPORT_PREFIX}/lib/kademlia-impl.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
