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
    ${CMAKE_SOURCE_DIR}/include/kademlia/session.hpp
    session.cpp
    ${CMAKE_SOURCE_DIR}/include/kademlia/endpoint.hpp
    endpoint.cpp
    ${CMAKE_SOURCE_DIR}/include/kademlia/error.hpp
    error.cpp
    boost_to_std_error.hpp
    id.cpp
    id.hpp
    message.cpp
    message.hpp
    message_serializer.cpp
    message_serializer.hpp
    message_socket.cpp
    message_socket.hpp
    response_dispatcher.cpp
    response_dispatcher.hpp
    routing_table.cpp
    routing_table.hpp
    value_context.hpp
    find_value_context.hpp
    store_value_context.hpp
    net.cpp
    net.hpp
    subnet.hpp
    timer.cpp
    timer.hpp
    value_store.hpp
    buffer.hpp)

add_library(kademlia SHARED
    ${kademlia_sources})

set_target_properties(kademlia 
    PROPERTIES 
        COMPILE_DEFINITIONS "KADEMLIA_BUILDING_AS_SHARED;KADEMLIA_AS_SHARED")

add_library(kademlia_convenience STATIC
    ${kademlia_sources})

target_link_libraries(kademlia ${Boost_LIBRARIES} ${BOTAN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

