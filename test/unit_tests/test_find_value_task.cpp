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
    k::tests::tracker_mock tracker_;
    std::error_code failure_;
    data_type data_;
};

} // anonymous namespace

BOOST_FIXTURE_TEST_SUITE( test_usage, fixture )

BOOST_AUTO_TEST_CASE( can_notify_error_when_routing_table_is_empty )
{
    k::tests::routing_table_mock routing_table{ kd::id{ "a" } };

    auto callback = [ this ]
        ( std::error_code const& f, data_type const& d )
    { failure_ = f; data_ = d; };

    BOOST_REQUIRE( ! routing_table.find_called_ );

    kd::start_find_value_task< data_type >( routing_table.expected_key_
                                          , tracker_
                                          , routing_table
                                          , callback );

    BOOST_REQUIRE( routing_table.find_called_ );
    BOOST_REQUIRE( failure_ == k::VALUE_NOT_FOUND );
    BOOST_REQUIRE( ! tracker_.has_sent_message() );
}

BOOST_AUTO_TEST_CASE( can_issue_find_value )
{
    kd::id searched_id{ "a" };
    k::tests::routing_table_mock routing_table{ searched_id };
    auto e1 = kd::to_ip_endpoint( "192.168.1.1", 5555 );
    k::tests::routing_table_mock::peer_type p1{ kd::id{ "b" }, e1 };
    routing_table.peers_.push_back( p1 );

    auto callback = [ this ]
        ( std::error_code const& f, data_type const& d )
    { failure_ = f; data_ = d; };

    BOOST_REQUIRE( ! routing_table.find_called_ );

    kd::start_find_value_task< data_type >( routing_table.expected_key_
                                          , tracker_
                                          , routing_table
                                          , callback );

    BOOST_REQUIRE( routing_table.find_called_ );
    BOOST_REQUIRE( ! failure_ );
    BOOST_REQUIRE( tracker_.has_sent_message( e1
                                            , kd::header{ kd::header::V1
                                                        , kd::header::FIND_VALUE_REQUEST
                                                        , kd::id{}
                                                        , kd::id{} }
                                            , kd::find_value_request_body{ searched_id } ) );
}


BOOST_AUTO_TEST_SUITE_END()

