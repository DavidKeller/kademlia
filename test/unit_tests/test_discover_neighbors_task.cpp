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

struct fixture : k::tests::task_fixture
{
    void
    operator()
        ( std::error_code const& failure )
    { failure_ = failure; }
};

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
                                     , endpoints
                                     , std::ref( *this ) );

    io_service_.poll();

    // As neither of theses addresses responded, the task throws.
    BOOST_REQUIRE( failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND );
}

BOOST_AUTO_TEST_CASE( can_contact_endpoints_until_one_respond )
{
    kd::id const my_id{ "a" };

    // Assume initial peer resolves to 2 IPv4 addresses.
    auto const e1 = kd::to_ip_endpoint( "192.168.1.2", 5555 );
    auto const e2 = kd::to_ip_endpoint( "192.168.1.3", 5555 );
    auto const e3 = kd::to_ip_endpoint( "::4", 5555 );
    endpoints_type const endpoints{ e1, e2, e3 };

    tracker_.add_message_to_receive( e2
                                   , my_id
                                   , kd::find_peer_response_body{} );

    kd::start_discover_neighbors_task( my_id 
                                     , tracker_
                                     , routing_table_
                                     , endpoints
                                     , std::ref( *this ) );

    io_service_.poll();

    kd::find_peer_request_body const fp{ my_id };
    // Task queried e3 but it didn't respond.
    BOOST_REQUIRE( tracker_.has_sent_message( e3, fp ) );

    // Hence task queried e2 that responded without
    // providing new peer.
    BOOST_REQUIRE( tracker_.has_sent_message( e2, fp ) );

    // Task didn't send any more message as previous peer responded.
    BOOST_REQUIRE( ! tracker_.has_sent_message() );

    BOOST_REQUIRE( ! failure_ );
}

BOOST_AUTO_TEST_SUITE_END()

