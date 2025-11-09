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

add_executable(kademlia-unit-tests
    main.cpp
    common.cpp
    network_utils.cpp
    test_boost_to_std_error.cpp
    test_concurrent_guard.cpp
    test_discover_neighbors_task.cpp
    test_endpoint.cpp
    test_engine.cpp
    test_error.cpp
    test_fake_socket.cpp
    test_find_value_task.cpp
    test_first_session.cpp
    test_id.cpp
    test_ip_endpoint.cpp
    test_log.cpp
    test_lookup_task.cpp
    test_message_serializer.cpp
    test_message_socket.cpp
    test_message.cpp
    test_network.cpp
    test_notify_peer_task.cpp
    test_peer.cpp
    test_r.cpp
    test_response_callbacks.cpp
    test_response_router.cpp
    test_routing_table.cpp
    test_session.cpp
    test_store_value_task.cpp
    test_timer.cpp
)
target_compile_definitions(kademlia-unit-tests
    PRIVATE
        TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(kademlia-unit-tests
    PRIVATE
        Boost::unit_test_framework
        Boost::filesystem
        kademlia-impl
        kademlia-test
)
add_test(unit-tests kademlia-unit-tests)