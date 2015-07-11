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
#include "kademlia/find_value_task.hpp"

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
        , data_()
        , routing_table_()
    { }

    void
    operator()
        ( std::error_code const& f
        , data_type const& d )
    { failure_ = f; data_ = d; }

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
    data_type data_;
    k::tests::routing_table_mock routing_table_;
};

} // anonymous namespace

BOOST_FIXTURE_TEST_SUITE( test_usage, fixture )

BOOST_AUTO_TEST_CASE( can_notify_error_when_routing_table_is_empty )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    BOOST_REQUIRE_EQUAL( 0, routing_table_.find_call_count_ );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );

    io_service_.poll();

    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
    BOOST_REQUIRE( ! tracker_.has_sent_message() );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_unique_peer_fails_to_respond )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "a" } );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second
                                            , kd::find_value_request_body{ searched_key } ) );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_all_peers_fail_to_respond )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    auto p2 = add_peer( "192.168.1.2", kd::id{ "c" } );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second
                                            , kd::find_value_request_body{ searched_key } ) );
    BOOST_REQUIRE( tracker_.has_sent_message( p2.second
                                            , kd::find_value_request_body{ searched_key } ) );
}

BOOST_AUTO_TEST_CASE( can_notify_error_when_no_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    auto p2 = add_peer( "192.168.1.2", kd::id{ "c" } );

    tracker_.add_message_to_receive( p1.second
                                   , p1.first
                                   , kd::find_peer_response_body{} );
    tracker_.add_message_to_receive( p2.second
                                   , p2.first
                                   , kd::find_peer_response_body{} );

    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second
                                            , kd::find_value_request_body{ searched_key } ) );
    BOOST_REQUIRE( tracker_.has_sent_message( p2.second
                                            , kd::find_value_request_body{ searched_key } ) );
}

BOOST_AUTO_TEST_CASE( can_return_value_when_already_known_peer_has_the_value )
{
    kd::id const searched_key{ "a" };
    routing_table_.expected_ids_.emplace_back( searched_key );

    auto p1 = add_peer( "192.168.1.1", kd::id{ "b" } );
    kd::find_value_response_body const b1{ { 1, 2, 3, 4 } };
    tracker_.add_message_to_receive( p1.second, p1.first, b1 );
    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );
    BOOST_REQUIRE( ! failure_ );
    BOOST_REQUIRE_EQUAL_COLLECTIONS( b1.data_.begin(), b1.data_.end()
                                   , data_.begin(), data_.end() );
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second
                                            , kd::find_value_request_body{ searched_key } ) );
}

BOOST_AUTO_TEST_CASE( can_return_value_when_unknown_peer_has_the_value )
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
    kd::find_value_response_body const fv2{ { 1, 2, 3, 4 } };
    tracker_.add_message_to_receive( e2, i2, fv2 );
    kd::start_find_value_task< data_type >( searched_key
                                          , tracker_
                                          , routing_table_
                                          , std::ref( *this ) );
    io_service_.poll();

    BOOST_REQUIRE_EQUAL( 1, routing_table_.find_call_count_ );
    BOOST_REQUIRE( ! failure_ );
    BOOST_REQUIRE_EQUAL_COLLECTIONS( fv2.data_.begin(), fv2.data_.end()
                                   , data_.begin(), data_.end() );
    BOOST_REQUIRE( tracker_.has_sent_message( p1.second
                                            , kd::find_value_request_body{ searched_key } ) );
    BOOST_REQUIRE( tracker_.has_sent_message( e2
                                            , kd::find_value_request_body{ searched_key } ) );
}

BOOST_AUTO_TEST_SUITE_END()

