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

# Run all tests, timeouting if it takes more than 30 seconds.
add_custom_target(unit_tests COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --timeout 30)
add_dependencies(check unit_tests)

find_package(GTest REQUIRED)

include_directories(BEFORE .)

add_library(test_helpers STATIC
    common.cpp
    common.hpp
    network.hpp
    network.cpp
    peer_factory.hpp
    routing_table_mock.hpp
    tracker_mock.hpp
    socket_mock.hpp
    corrupted_message.hpp
    task_fixture.hpp)

target_link_libraries(test_helpers
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${GTEST_BOTH_LIBRARIES})

set_target_properties(test_helpers PROPERTIES
    COMPILE_DEFINITIONS TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

macro(build_test test_name)
    cmake_parse_arguments(ARG "" "" "LIBRARIES;SOURCES" ${ARGN})
    add_executable(${test_name} ${ARG_SOURCES})
    target_link_libraries(${test_name}
        test_helpers
        ${ARG_LIBRARIES})
    add_dependencies(unit_tests ${test_name})
    add_test(${test_name} ${test_name})
endmacro()

build_test(unit_tests_helpers
    SOURCES
        test_fake_socket.cpp
    LIBRARIES
        kademlia_static)

build_test(unit_tests_lib
    SOURCES
        test_id.cpp
        test_endpoint.cpp
        test_boost_to_std_error.cpp
        test_message.cpp
        test_message_serializer.cpp
        test_lookup_task.cpp
        test_store_value_task.cpp
        test_find_value_task.cpp
        test_ip_endpoint.cpp
        test_peer.cpp
        test_error.cpp
        test_discover_neighbors_task.cpp
        test_notify_peer_task.cpp
        test_response_router.cpp
        test_response_callbacks.cpp
        test_timer.cpp
        test_network.cpp
        test_message_socket.cpp
        test_log.cpp
        test_r.cpp
        test_routing_table.cpp
        test_session.cpp
        test_first_session.cpp
        test_concurrent_guard.cpp
        test_engine.cpp
        test_fake_socket.cpp
    LIBRARIES 
        kademlia_static)

if(ENABLE_COVERAGE)
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        if(${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
            find_program(XCRUN xcrun)
            if(XCRUN)
                execute_process(COMMAND
                                    ${XCRUN} -f llvm-cov
                                OUTPUT_VARIABLE
                                    LLVM_COV
                                OUTPUT_STRIP_TRAILING_WHITESPACE)
            endif()
        else()
            find_program(LLVM_COV llvm-cov)
        endif()

        if (NOT LLVM_COV)
            message(FATAL_ERROR "Can't find llvm-cov tool")
        endif()

        set(GCOV ${CMAKE_CURRENT_BINARY_DIR}/llvm_gcov)
        configure_file(llvm_gcov.in ${GCOV} @ONLY)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        find_program(GCOV gcov)

        if (NOT GCOV)
            message(FATAL_ERROR "Can't find gcov tool")
        endif()
    endif()

    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    if (NOT LCOV OR NOT GENHTML)
        message(FATAL_ERROR "Can't find lcov tool(s)")
    endif()

    add_custom_target(coverage
        COMMAND
            ${LCOV} --quiet --output-file app_base.info --initial --capture --directory . --gcov-tool ${GCOV}
        COMMAND
            ${CMAKE_COMMAND} --build . --target unit_tests
        COMMAND
            ${LCOV} --quiet --output-file app_test.info --capture --directory . --gcov-tool ${GCOV}
        COMMAND
            ${LCOV} --quiet --output-file app_total.info --add-tracefile app_base.info --add-tracefile app_test.info
        COMMAND
            ${LCOV} --quiet --output-file app_total_stripped.info --extract app_total.info "*/include/kademlia/*" "*/src/kademlia/*"
        COMMAND
            ${GENHTML} --output-directory html --num-spaces 4 --title "Kademlia unit tests" --demangle-cpp --no-function-coverage --prefix ${CMAKE_SOURCE_DIR} app_total_stripped.info
        WORKING_DIRECTORY
            ${CMAKE_BINARY_DIR}
        VERBATIM)
    add_dependencies(coverage kademlia_static)
endif()

