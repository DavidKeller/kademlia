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

#include <vector>
#include <utility>

#include "kademlia/id.hpp"
#include "kademlia/peer.hpp"
#include "kademlia/ip_endpoint.hpp"

#include "kademlia/store_value_task.hpp"

namespace k = kademlia;
namespace kd = k::detail;

namespace {

using data_type = std::vector< std::uint8_t >;

struct fixture
{
    fixture
        ( void )
        : io_service_()
        , io_service_work_( io_service_ )
        , tracker_( io_service_ )
        , failure_()
        , routing_table_()
        , callback_call_count_()
    { }

    void
    operator()
        ( std::error_code const& f )
    {
        ++ callback_call_count_;
        failure_ = f;
    }

    k::tests::routing_table_mock::peer_type
    add_peer
        ( std::string const& ip, kd::id const& id )
    {
        auto e = kd::to_ip_endpoint( ip, 5555 );
        k::tests::routing_table_mock::peer_type p{ id, e };
        routing_table_.peers_.push_back( p );

        return p;
    }

    boost::asio::io_service io_service_;
    boost::asio::io_service::work io_service_work_;
    k::tests::tracker_mock tracker_;
    std::error_code failure_;
    k::tests::routing_table_mock routing_table_;
    std::size_t callback_call_count_;
};

} // anonymous namespace

BOOST_FIXTURE_TEST_SUITE( test_usage, fixture )

BOOST_AUTO_TEST_CASE( can_notify_error_when_routing_table_is_empty )
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back( chosen_key );

    BOOST_REQUIRE_EQUAL( 0, routing_table_.find_call_count_ );

    kd::start_store_value_task< data_type >( chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref( *this ) );

    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task didn't send any more message.
    BOOST_REQUIRE( failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND );

    // Task notified the error.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_unique_peer_fails_to_respond )
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back( chosen_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );

    kd::start_store_value_task< data_type >( chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the error.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_all_peers_fail_to_respond )
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back( chosen_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    auto p2 = add_peer( "192.168.1.2", kd::id{ "c" } );

    kd::start_store_value_task< data_type >( chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 & p2 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );
    BOOST_REQUIRE( tracker_.has_sent_message( p2.second, fv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the error.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND );
}

BOOST_AUTO_TEST_CASE( can_store_value_when_already_known_peer_is_the_target )
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back( chosen_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    kd::find_peer_response_body const b1{};
    tracker_.add_message_to_receive( p1.second, p1.first, b1 );
    kd::start_store_value_task< data_type >( chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );

    // Task decided that p1 was the closest
    // hence it asked to store data on it.
    kd::store_value_request_body const sv{ chosen_key, data };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, sv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the success.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( ! failure_ );
}

BOOST_AUTO_TEST_CASE( can_store_value_when_discovered_peer_is_the_target )
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back( chosen_key );

    // p1 is the only known peer atm.
    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );

    // p2 is unknown atm.
    auto i2 = kd::id{ chosen_key };
    auto e2 = kd::to_ip_endpoint( "192.168.1.2", 5555 );
    kd::peer const p2{ i2, e2 };

    // p1 knows p2.
    kd::find_peer_response_body const fp1{ { p2 } };
    tracker_.add_message_to_receive( p1.second, p1.first, fp1 );

    // And p2 doesn't know closer peer.
    kd::find_peer_response_body const fp2{};
    tracker_.add_message_to_receive( e2, i2, fp2 );

    kd::start_store_value_task< data_type >( chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref( *this ) );
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );

    // Task asked p1 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, fv ) );

    // Task then asked p2 for a closer peer.
    BOOST_REQUIRE( tracker_.has_sent_message( e2, fv ) );

    // Task decided that p2 was the closest
    // hence it asked to store data on it.
    kd::store_value_request_body const sv{ chosen_key, data };
    BOOST_REQUIRE( tracker_.has_sent_message( e2, sv ) );

    // Task is also required to store data 
    // on close peers for redundancy purpose.
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second, sv ) );

    // Task didn't send any more message.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    // Task notified the success.
    BOOST_REQUIRE_EQUAL( 1, callback_call_count_ );
    BOOST_REQUIRE( ! failure_ );
}

BOOST_AUTO_TEST_SUITE_END()

