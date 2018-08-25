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

#include "common.hpp"
#include "peer_factory.hpp"

#include <vector>
#include <utility>

#include "kademlia/id.hpp"
#include "kademlia/message_socket.hpp"
#include "kademlia/lookup_task.hpp"

namespace {

namespace k = kademlia;
namespace kd = k::detail;

struct test_task : kd::lookup_task {
    template< typename Iterator >
    test_task
        ( kd::id const& key
        , Iterator i, Iterator e )
        : lookup_task{ key, i, e }
    { }
};

using routing_table_peer = std::pair< kd::id
                                    , kd::ip_endpoint >;

BOOST_AUTO_TEST_SUITE( lookup_task )

BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( can_be_constructed_without_candidates )
{
    std::vector< routing_table_peer > candidates;
    kd::id const key{};
    test_task c{ key, candidates.begin(), candidates.end() };

    BOOST_REQUIRE( c.have_all_requests_completed() );
    BOOST_REQUIRE_EQUAL( 0, c.select_new_closest_candidates( 1 ).size() );
    BOOST_REQUIRE_EQUAL( 0, c.select_closest_valid_candidates( 1 ).size() );
    BOOST_REQUIRE( c.have_all_requests_completed() );
    BOOST_REQUIRE_EQUAL( key, c.get_key() );
}

BOOST_AUTO_TEST_CASE( can_be_constructed_with_candidates )
{
    std::vector< routing_table_peer > candidates;
    kd::ip_endpoint const default_address{};
    candidates.emplace_back( kd::id{ "1" }, default_address );
    kd::id const key{};
    test_task c{ key, candidates.begin(), candidates.end() };
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( test_usage )

BOOST_AUTO_TEST_CASE( can_select_candidates )
{
    std::vector< routing_table_peer > candidates;
    kd::ip_endpoint const default_address{};
    candidates.emplace_back( kd::id{ "7" }, default_address );
    candidates.emplace_back( kd::id{ "3" }, default_address );
    candidates.emplace_back( kd::id{ "6" }, default_address );
    candidates.emplace_back( kd::id{ "18" }, default_address );
    candidates.emplace_back( kd::id{ "2" }, default_address );
    candidates.emplace_back( kd::id{ "9" }, default_address );
    candidates.emplace_back( kd::id{ "1" }, default_address );
    kd::id const key{};
    test_task c{ key, candidates.begin(), candidates.end() };

    BOOST_REQUIRE( c.have_all_requests_completed() );
    auto closest_candidates = c.select_new_closest_candidates( 2 );
    BOOST_REQUIRE( ! c.have_all_requests_completed() );
    BOOST_REQUIRE_EQUAL( 2, closest_candidates.size() );
    BOOST_REQUIRE_EQUAL( kd::id{ "1" }, closest_candidates[ 0 ].id_ );
    BOOST_REQUIRE_EQUAL( kd::id{ "2" }, closest_candidates[ 1 ].id_ );

    BOOST_REQUIRE_EQUAL( 0, c.select_closest_valid_candidates( 1 ).size() );
    BOOST_REQUIRE( ! c.have_all_requests_completed() );
    c.flag_candidate_as_valid( kd::id{ "1" } );
    auto const valid_candidates = c.select_closest_valid_candidates( 1 );
    BOOST_REQUIRE_EQUAL( 1, valid_candidates.size() );
    BOOST_REQUIRE_EQUAL( kd::id{ "1" }, valid_candidates[ 0 ].id_ );
    BOOST_REQUIRE( ! c.have_all_requests_completed() );

    c.flag_candidate_as_invalid( kd::id{ "2" } );
    BOOST_REQUIRE( c.have_all_requests_completed() );

    closest_candidates = c.select_new_closest_candidates( 2 );
    BOOST_REQUIRE( ! c.have_all_requests_completed() );
    BOOST_REQUIRE_EQUAL( 2, closest_candidates.size() );
    BOOST_REQUIRE_EQUAL( kd::id{ "3" }, closest_candidates[ 0 ].id_ );
    BOOST_REQUIRE_EQUAL( kd::id{ "6" }, closest_candidates[ 1 ].id_ );
}

BOOST_AUTO_TEST_CASE( can_add_candidates )
{
    std::vector< routing_table_peer > candidates;
    kd::ip_endpoint const default_address{};
    candidates.emplace_back( kd::id{ "7" }, default_address );

    kd::id const our_id{};
    test_task c{ our_id, candidates.begin(), candidates.end() };

    BOOST_REQUIRE_EQUAL( 1, c.select_new_closest_candidates( 20 ).size() );

    std::vector< kd::peer > new_candidates;

    new_candidates.emplace_back( create_peer( kd::id{ "7" } ) );
    c.add_candidates( new_candidates );
    BOOST_REQUIRE_EQUAL( 0, c.select_new_closest_candidates( 20 ).size() );

    new_candidates.emplace_back( create_peer( kd::id{ "6" } ) );
    new_candidates.emplace_back( create_peer( kd::id{ "8" } ) );
    c.add_candidates( new_candidates );
    BOOST_REQUIRE_EQUAL( 2, c.select_new_closest_candidates( 20 ).size() );

    candidates.clear();
    new_candidates.emplace_back( create_peer( kd::id{ "9" } ) );
    c.add_candidates( new_candidates );
    BOOST_REQUIRE_EQUAL( 1, c.select_new_closest_candidates( 20 ).size() );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

