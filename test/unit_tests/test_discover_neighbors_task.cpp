// Copyright (c) 2013-2014, David Keller
// All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the University of California, Berkeley nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY DAVID KELLER AND CONTRIBUTORS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "helpers/common.hpp"
#include "helpers/tracker_mock.hpp"
#include "helpers/routing_table_mock.hpp"
#include "helpers/task_fixture.hpp"

#include <vector>
#include <utility>

#include "kademlia/id.hpp"
#include "kademlia/peer.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/discover_neighbors_task.hpp"

namespace k = kademlia;
namespace kd = k::detail;

namespace {

using endpoints_type = std::vector< kd::ip_endpoint >;

using fixture = k::tests::task_fixture;

} // anonymous namespace

BOOST_FIXTURE_TEST_SUITE( test_usage, fixture )

BOOST_AUTO_TEST_CASE( can_notify_error_when_initial_endpoints_fail_to_respond )
{
    kd::id const my_id{ "a" };

    // Assume initial peer resolves to 2 IPv4 addresses.
    endpoints_type const endpoints{ kd::to_ip_endpoint( "192.168.1.2", 5555 )
                                  , kd::to_ip_endpoint( "192.168.1.3", 5555 ) };

    kd::start_discover_neighbors_task( my_id 
                                     , tracker_
                                     , routing_table_
                                     , endpoints );

    // As neither of theses addresses responded, the task throws.
    BOOST_REQUIRE_THROW( io_service_.poll(), std::runtime_error );
}

#if 0

BOOST_AUTO_TEST_CASE( can_notify_error_when_unique_peer_fails_to_respond )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "a" } );

    kd::start_discover_neighbors_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer or the value.
    kd::discover_neighbors_request_body const fv{ searched_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the error.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_all_peers_fail_to_respond )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    auto p2 = add_peer( "192.168.1.2", kd::id{ "c" } );

    kd::start_discover_neighbors_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 & p2 for a closer peer or the value.
    kd::discover_neighbors_request_body const fv{ searched_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );
    BOOST_REQUIRE( tracker_.has_sent_message( p2.second, fv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the error.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_no_already_known_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    auto p2 = add_peer( "192.168.1.2", kd::id{ "c" } );

    // p1 doesn't know closer peer.
    tracker_.add_message_to_receive( p1.second
                                   , p1.first
                                   , kd::find_peer_response_body{} );

    // p2 doesn't know closer peer.
    tracker_.add_message_to_receive( p2.second
                                   , p2.first
                                   , kd::find_peer_response_body{} );

    kd::start_discover_neighbors_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 & p2 for a closer peer or the value.
    kd::discover_neighbors_request_body const fv{ searched_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );
    BOOST_REQUIRE( tracker_.has_sent_message( p2.second, fv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the error.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_no_discovered_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    // p1 is the only known peer atm.
    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );

    // p2 is unknown atm.
    auto i2 = kd::id{ searched_key };
    auto e2 = kd::to_ip_endpoint( "192.168.1.2", 5555 );
    kd::peer const p2{ i2, e2 };

    // p1 knows p2.
    kd::find_peer_response_body const fp1{ { p2 } };
    tracker_.add_message_to_receive( p1.second, p1.first, fp1 );

    // p2 does'nt known closer peer nor has the value.
    tracker_.add_message_to_receive( e2
                                   , i2
                                   , kd::find_peer_response_body{} );

    kd::start_discover_neighbors_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 & p2 for a closer peer or the value.
    kd::discover_neighbors_request_body const fv{ searched_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );
    BOOST_REQUIRE( tracker_.has_sent_message( e2, fv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the error.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
}

BOOST_AUTO_TEST_CASE( can_return_value_when_already_known_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    kd::discover_neighbors_response_body const b1{ { 1, 2, 3, 4 } };
    tracker_.add_message_to_receive( p1.second, p1.first, b1 );
    kd::start_discover_neighbors_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer or the value.
    kd::discover_neighbors_request_body const fv{ searched_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the success.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( ! failure_ );
    BOOST_REQUIRE_EQUAL_COLLECTIONS( b1.data_.begin(), b1.data_.end()
                                   , data_.begin(), data_.end() );
}

BOOST_AUTO_TEST_CASE( can_return_value_when_discovered_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    // p1 is the only known peer atm.
    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );

    // p2 is unknown atm.
    auto i2 = kd::id{ searched_key };
    auto e2 = kd::to_ip_endpoint( "192.168.1.2", 5555 );
    kd::peer const p2{ i2, e2 };

    // p1 knows p2.
    kd::find_peer_response_body const fp1{ { p2 } };
    tracker_.add_message_to_receive( p1.second, p1.first, fp1 );

    // And p2 knows the value.
    kd::discover_neighbors_response_body const fv2{ { 1, 2, 3, 4 } };
    tracker_.add_message_to_receive( e2, i2, fv2 );
    kd::start_discover_neighbors_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer or the value.
    kd::discover_neighbors_request_body const fv{ searched_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );
    BOOST_REQUIRE( tracker_.has_sent_message( e2, fv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the success.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( ! failure_ );
    BOOST_REQUIRE_EQUAL_COLLECTIONS( fv2.data_.begin(), fv2.data_.end()
                                   , data_.begin(), data_.end() );
}

#endif

BOOST_AUTO_TEST_SUITE_END()

