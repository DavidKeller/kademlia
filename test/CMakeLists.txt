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

# Include implementation headers.
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src)

# Run all tests, timeouting if it takes more than 30 seconds.
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --timeout 30)

add_library(test_helpers_common STATIC
    helpers/common.cpp 
    helpers/common.hpp)

set_target_properties(test_helpers_common PROPERTIES 
    COMPILE_DEFINITIONS TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

macro(build_and_run_test source_file)
    get_filename_component(test_name ${source_file} NAME_WE)
    add_executable(${test_name} ${source_file} ${ARGN}) 
    target_link_libraries(${test_name} 
        test_helpers_common 
        kademlia_convenience 
        ${Boost_LIBRARIES} 
        ${OPENSSL_LIBRARIES} 
        ${CMAKE_THREAD_LIBS_INIT})
    add_dependencies(check ${test_name})
    add_test(${test_name} ${test_name})
endmacro()

build_and_run_test(test_id.cpp)
build_and_run_test(test_message.cpp)
build_and_run_test(test_value_context.cpp)
build_and_run_test(test_store_value_context.cpp)
build_and_run_test(test_find_value_context.cpp)
build_and_run_test(test_response_dispatcher.cpp)
build_and_run_test(test_timer.cpp)
build_and_run_test(test_message_socket.cpp)
build_and_run_test(test_r.cpp)
build_and_run_test(test_routing_table.cpp helpers/peer_factory.hpp)
build_and_run_test(test_session.cpp)

