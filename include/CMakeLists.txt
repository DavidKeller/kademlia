include(GenerateExportHeader)
generate_export_header(kademlia
  EXPORT_FILE_NAME
    kademlia/detail/symbol_visibility.hpp
)

target_sources(kademlia
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}"
      FILES
        "${CMAKE_CURRENT_BINARY_DIR}/kademlia/detail/symbol_visibility.hpp"
        kademlia/first_session.hpp
        kademlia/session_base.hpp
        kademlia/session.hpp
        kademlia/error.hpp
        kademlia/endpoint.hpp
)

target_include_directories(kademlia
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_include_directories(kademlia-impl
  INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)