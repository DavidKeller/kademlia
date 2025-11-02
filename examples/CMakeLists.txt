# SPDX-License-Identifier: MIT

add_executable(kademlia_cli
    cli.cpp)

target_link_libraries(kademlia_cli
    PRIVATE
        kademlia
        Threads::Threads)

add_executable(kademlia_bootstrap
    bootstrap.cpp)

target_link_libraries(kademlia_bootstrap
    PRIVATE
        kademlia
        Threads::Threads)
