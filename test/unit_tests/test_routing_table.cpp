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

#include "kademlia/routing_table.hpp"
#include "kademlia/ip_endpoint.hpp"

namespace {

namespace k = kademlia;
namespace kd = k::detail;

using test_routing_table = kd::routing_table< kd::ip_endpoint >;

BOOST_AUTO_TEST_SUITE( routing_table )

BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( is_empty_on_construction )
{
    std::default_random_engine random_engine;

    // Create an empty routing_table.
    test_routing_table rt{ kd::id( random_engine ) };
    // Doesn't contain any peer.
    BOOST_REQUIRE_EQUAL( rt.peer_count(), 0 );
}

BOOST_AUTO_TEST_SUITE_END()


/**
 *  Test test_routing_table::push()
 */
BOOST_AUTO_TEST_SUITE( test_push )

BOOST_AUTO_TEST_CASE( largest_k_bucket_can_receive_unlimited_peers )
{
    // My id is 128 bit assigned to 0.
    kd::id const my_id;
    // Each bucket can contain up to 4 peers.
    std::size_t const bucket_size = 2;

    test_routing_table rt{ my_id, bucket_size };

    // This peer will be associated with every id.
    // Unicity applies only to id, not peer.
    auto const test_peer( create_endpoint() );

    // Theses should go into high bucket.
    BOOST_REQUIRE( rt.push( kd::id{ "10" }, test_peer ) );
    BOOST_REQUIRE( rt.push( kd::id{ "11" }, test_peer ) );

    // While theses go to a lower bucket.
    BOOST_REQUIRE( rt.push( kd::id{ "20" }, test_peer ) );
    BOOST_REQUIRE( rt.push( kd::id{ "21" }, test_peer ) );
    // This one should flag the lower bucket as the largest.
    BOOST_REQUIRE( rt.push( kd::id{ "22" }, test_peer ) );
    // This one can be saved as the largest bucket
    // is allowed to exceed bucket_size.
    BOOST_REQUIRE( rt.push( kd::id{ "23" }, test_peer ) );

    // This one can't go into first bucket as it is full
    // and is not the largest bucket.
    BOOST_REQUIRE(! rt.push( kd::id{ "12" }, test_peer ) );
}

BOOST_AUTO_TEST_CASE( discards_already_pushed_ids )
{
    std::default_random_engine random_engine;

    test_routing_table rt{ kd::id{ random_engine } };
    auto const test_peer( create_endpoint() );
    kd::id test_id;

    // Push two times the same peer.
    BOOST_REQUIRE_EQUAL( rt.push( test_id, test_peer ), true );
    // Expect the second call to fail.
    BOOST_REQUIRE_EQUAL( rt.push( test_id, test_peer ), false );
    BOOST_REQUIRE_EQUAL( rt.peer_count(), 1 );
}

BOOST_AUTO_TEST_SUITE_END()

/**
 *  Test test_routing_table::find()
 */
BOOST_AUTO_TEST_SUITE( test_find )

BOOST_AUTO_TEST_CASE( can_find_a_peer )
{
    test_routing_table rt{ kd::id{} };
    auto test_peer( create_endpoint() );
    kd::id test_id{ "a" };
    BOOST_REQUIRE( rt.push( test_id, test_peer ) );

    // Try to find the generated peer.
    auto i = rt.find( test_id );
    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer, i->second );
}

BOOST_AUTO_TEST_CASE( can_find_a_closer_peer )
{
    test_routing_table rt{ kd::id{} };

    auto test_peer1( create_endpoint() );
    kd::id test_id1{ "1" };
    BOOST_REQUIRE( rt.push( test_id1, test_peer1 ) );

    auto test_peer2( create_endpoint() );
    kd::id test_id2{ "2" };
    BOOST_REQUIRE( rt.push( test_id2, test_peer2 ) );

    auto test_peer3( create_endpoint() );
    kd::id test_id3{ "4" };
    BOOST_REQUIRE( rt.push( test_id3, test_peer3 ) );

    // test_peer2 is the closest peer.
    auto i = rt.find( kd::id{ "6" } );
    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer2, i->second );
}

BOOST_AUTO_TEST_CASE( iterator_start_from_the_closest_k_bucket )
{
    test_routing_table rt( kd::id{}, 1 );
    auto test_peer1( create_endpoint( "192.168.0.1" ) );
    kd::id id1{ "1" };
    BOOST_REQUIRE( rt.push( id1, test_peer1 ) );

    auto test_peer2( create_endpoint( "192.168.0.2" ) );
    kd::id id2{ "2" };
    BOOST_REQUIRE( rt.push( id2, test_peer2 ) );

    auto test_peer3( create_endpoint( "192.168.0.3" ) );
    kd::id id3{ "4" };
    BOOST_REQUIRE( rt.push( id3, test_peer3 ) );

    // Ask for id of the closest peer, and expect to see
    // all of them.
    auto i = rt.find( id1 );

    // This one should be in the close bucket.
    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer1, i->second );
    ++ i;

    // This one too.
    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer2, i->second );
    ++ i;

    // This one in the far bucket.
    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer3, i->second );
    ++ i;

    BOOST_REQUIRE( i == rt.end() );
}

BOOST_AUTO_TEST_CASE( iterator_skip_empty_k_bucket )
{
    test_routing_table rt{ kd::id{}, 1 };
    // Fill far k_bucket.
    auto test_peer1( create_endpoint( "192.168.0.1" ) );
    kd::id id1{ "1" };
    BOOST_REQUIRE( rt.push( id1, test_peer1 ) );

    // Skip the next "2".

    // End with this one.
    auto test_peer2( create_endpoint( "192.168.0.2" ) );
    kd::id id2{ "4" };
    BOOST_REQUIRE( rt.push( id2, test_peer2 ) );

    // Ask for id of the closest peer, and expect to see
    // all of them.
    auto i = rt.find( id1 );

    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer1, i->second );
    ++ i;

    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer2, i->second );
    ++ i;

    BOOST_REQUIRE( i == rt.end() );
}

BOOST_AUTO_TEST_SUITE_END()

/**
 *  Test test_routing_table::remove()
 */
BOOST_AUTO_TEST_SUITE( test_remove )

BOOST_AUTO_TEST_CASE( can_remove_a_peer )
{
    test_routing_table rt{ kd::id{} };
    auto test_peer( create_endpoint() );
    kd::id test_id{};
    BOOST_REQUIRE( rt.push( test_id, test_peer ) );

    // Try to find the generated peer.
    BOOST_REQUIRE( rt.find( test_id ) != rt.end() );
    std::size_t saved_table_size = rt.peer_count();
    BOOST_REQUIRE_EQUAL( rt.remove( test_id ), true );
    BOOST_REQUIRE_EQUAL( rt.peer_count(), saved_table_size - 1 );
    BOOST_REQUIRE( rt.find( test_id ) == rt.end() );
}

BOOST_AUTO_TEST_SUITE_END()

/**
 *  Test operator<<()
 */
BOOST_AUTO_TEST_SUITE( test_print )

BOOST_AUTO_TEST_CASE( print_empty_test_routing_table )
{
    boost::test_tools::output_test_stream out( k::test::get_capture_path( "pattern_empty_routing_table.out" ), true);

    out << test_routing_table{ kd::id{}, 20 };

    BOOST_REQUIRE( out.match_pattern() );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

