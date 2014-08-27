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
#include "helpers/peer_factory.hpp"

#include "kademlia/routing_table.hpp"
#include "kademlia/ip_endpoint.hpp"

namespace k = kademlia;
namespace kd = k::detail;

using routing_table = kd::routing_table< kd::ip_endpoint >;

/**
 *  Test routing_table::routing_table()
 */
BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( is_empty_on_construction )
{
    std::default_random_engine random_engine;

    // Create an empty routing_table.
    routing_table rt{ kd::id( random_engine ) };
    // Doesn't contain any peer.
    BOOST_REQUIRE_EQUAL( rt.peer_count(), 0 );
}

BOOST_AUTO_TEST_SUITE_END()


/**
 *  Test routing_table::push()
 */
BOOST_AUTO_TEST_SUITE( test_push )

BOOST_AUTO_TEST_CASE( fill_buckets_and_split_them )
{
    // My id is 128 bit assigned to 0.
    kd::id const my_id;
    // Each bucket can contain up to 4 peers.
    std::size_t const bucket_size = 3;

    routing_table rt{ my_id, bucket_size };

    // This peer will be associated with every id.
    // Unicity applies only to id, not peer.
    auto const test_peer( create_endpoint() );

    // FIXME: Okay, not my best test.
    for ( std::size_t bucket_index = 0
        ; bucket_index < kd::id::BIT_SIZE
        ; ++ bucket_index )
    {
        kd::id current_id;
        // Fill 'current_depth' nth bucket.
        for ( auto position = bucket_index; position < kd::id::BIT_SIZE; ++ position )
        {
            // bucket_index => number of lower bit set to 0.
            // First iteration with depth equal to 2 : 00..0100
            // Then : 00..1100
            // Until: 11..1100
            current_id[ position ] = 1;

            std::size_t const index_in_new_bucket = position - bucket_index;
            // Until we reach current bucket limit, expect to new peers
            // to be added.
            if ( index_in_new_bucket < bucket_size )
                BOOST_REQUIRE_MESSAGE
                        ( rt.push( current_id, test_peer ) == true
                        , position << " nth peer in bucket "
                            << bucket_index << " should have been accepted" );
            // Now peers should be droped.
            else
                BOOST_REQUIRE_MESSAGE
                        ( rt.push( current_id, test_peer ) == false
                        , position << " nth peer in bucket "
                            << bucket_index << " should have been kicked" );
        }
    }
}

BOOST_AUTO_TEST_CASE( discards_already_pushed_ids )
{
    std::default_random_engine random_engine;

    routing_table rt{ kd::id{ random_engine } };
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
 *  Test routing_table::find()
 */
BOOST_AUTO_TEST_SUITE( test_find )

BOOST_AUTO_TEST_CASE( can_find_a_peer )
{
    routing_table rt{ kd::id{} };
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
    routing_table rt{ kd::id{} };

    auto test_peer1( create_endpoint() );
    kd::id test_id1{ "1" };
    BOOST_REQUIRE( rt.push( test_id1, test_peer1 ) );

    auto test_peer2( create_endpoint() );
    kd::id test_id2{ "2" };
    BOOST_REQUIRE( rt.push( test_id2, test_peer2 ) );

    auto test_peer3( create_endpoint() );
    kd::id test_id3{ "4" };
    BOOST_REQUIRE( rt.push( test_id3, test_peer3 ) );

    // test_peer2 is the closer peer.
    auto i = rt.find( kd::id{ "6" } );
    BOOST_REQUIRE( i != rt.end() );
    BOOST_REQUIRE_EQUAL( test_peer2, i->second );
}

BOOST_AUTO_TEST_CASE( iterator_start_from_the_closest_k_bucket )
{
    routing_table rt( kd::id{}, 1 );
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
    routing_table rt{ kd::id{}, 1 };
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
 *  Test routing_table::remove()
 */
BOOST_AUTO_TEST_SUITE( test_remove )

BOOST_AUTO_TEST_CASE( can_remove_a_peer )
{
    routing_table rt{ kd::id{} };
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

BOOST_AUTO_TEST_CASE( print_empty_routing_table )
{
    boost::test_tools::output_test_stream out( get_capture_path( "pattern_empty_routing_table.out" ), true);

    out << routing_table{ kd::id{}, 20 };

    BOOST_REQUIRE( out.match_pattern() );
}

BOOST_AUTO_TEST_SUITE_END()

