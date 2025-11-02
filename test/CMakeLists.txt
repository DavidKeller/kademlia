# SPDX-License-Identifier: MIT

add_library(kademlia-test INTERFACE)

target_include_directories(kademlia-test
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(kademlia-test
    INTERFACE
        kademlia-impl
)

add_subdirectory(unit_tests)