# Copyright (c) 2013, David Keller
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the University of California, Berkeley nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY DAVID KELLER AND CONTRIBUTORS ``AS IS AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

set(kademlia_sources
    ${CMAKE_SOURCE_DIR}/include/kademlia/endpoint.hpp
    ${CMAKE_SOURCE_DIR}/include/kademlia/error.hpp
    ${CMAKE_SOURCE_DIR}/include/kademlia/session_base.hpp
    ${CMAKE_SOURCE_DIR}/include/kademlia/first_session.hpp
    ${CMAKE_SOURCE_DIR}/include/kademlia/session.hpp
    session_impl.hpp
    boost_to_std_error.hpp
    buffer.hpp
    concurrent_guard.hpp
    constants.cpp
    constants.hpp
    endpoint.cpp
    engine.hpp
    error.cpp
    error_impl.hpp
    error_impl.cpp
    find_value_task.hpp
    id.cpp
    id.hpp
    ip_endpoint.cpp
    ip_endpoint.hpp
    log.cpp
    log.hpp
    network.hpp
    message.cpp
    message.hpp
    message_serializer.cpp
    message_serializer.hpp
    message_socket.hpp
    peer.cpp
    peer.hpp
    response_callbacks.cpp
    response_callbacks.hpp
    response_router.hpp
    routing_table.hpp
    session.cpp
    session_base.cpp
    first_session.cpp
    store_value_task.hpp
    discover_neighbors_task.hpp
    timer.cpp
    timer.hpp
    tracker.hpp
    value_store.hpp
    lookup_task.hpp)

# Kademlia shared
add_library(kademlia SHARED
    ${kademlia_sources})

set_target_properties(kademlia
    PROPERTIES
        MACOSX_RPATH ON
        COMPILE_DEFINITIONS "KADEMLIA_BUILDING_AS_SHARED;KADEMLIA_AS_SHARED")

target_link_libraries(kademlia
    ${Boost_SYSTEM_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT})

# Kademlia static
add_library(kademlia_static STATIC
    ${kademlia_sources})

target_link_libraries(kademlia_static
    ${Boost_SYSTEM_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(kademlia_static
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON)

