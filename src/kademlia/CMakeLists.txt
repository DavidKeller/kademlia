# SPDX-License-Identifier: MIT
add_library(kademlia 
    first_session.cpp
    session.cpp
    session_base.cpp
    error.cpp
    endpoint.cpp
    session_impl.hpp
    buffer.hpp
    constants.cpp
    engine.hpp
    id.cpp
    ip_endpoint.cpp
    log.cpp
    message.cpp
    message_serializer.cpp
    peer.cpp
    response_callbacks.cpp
    timer.cpp
)

target_link_libraries(kademlia
    PRIVATE
        Boost::headers
        Boost::system
        OpenSSL::Crypto
        Threads::Threads
)

add_library(${PROJECT_NAME}::kademlia
    ALIAS
        kademlia
)

add_library(kademlia-impl INTERFACE)
target_include_directories(
  kademlia-impl
  INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_sources(kademlia-impl
    INTERFACE
        $<TARGET_OBJECTS:kademlia>
)
target_link_libraries(kademlia-impl
    INTERFACE
        Boost::headers
        Boost::system
        OpenSSL::Crypto
        Threads::Threads
)