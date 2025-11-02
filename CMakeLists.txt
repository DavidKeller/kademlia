# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.27)

project(kademlia LANGUAGES CXX VERSION 0.0.2)

include(cmake/options.cmake)
include(cmake/dependencies.cmake)
include(cmake/project_setup.cmake)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(test)
add_subdirectory(examples)
if(KADEMLIA_ENABLE_DOCUMENTATION)
    add_subdirectory(docs)
endif()

include(cmake/install.cmake)