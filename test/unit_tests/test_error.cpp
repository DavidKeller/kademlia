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

#include <kademlia/error.hpp>
#include <algorithm>
#include <cctype>

#include "common.hpp"

namespace {

namespace k = kademlia;

bool
compare_enum_to_message( char const * name, k::error_type const& error )
{
    auto message = make_error_condition( error ).message();

    std::replace( message.begin(), message.end(), ' ', '_' );
    std::transform( message.begin()
                  , message.end(), message.begin(),
                  ::toupper );

    return name == message;
}

BOOST_AUTO_TEST_SUITE( error )

BOOST_AUTO_TEST_SUITE( test_usage )

#define KADEMLIA_TEST_ERROR( e ) \
    BOOST_REQUIRE( compare_enum_to_message( #e, k:: e ) )

BOOST_AUTO_TEST_CASE( error_message_follows_the_error_name )
{
    KADEMLIA_TEST_ERROR( UNKNOWN_ERROR );
    KADEMLIA_TEST_ERROR( RUN_ABORTED );
    KADEMLIA_TEST_ERROR( INITIAL_PEER_FAILED_TO_RESPOND );
    KADEMLIA_TEST_ERROR( MISSING_PEERS );
    KADEMLIA_TEST_ERROR( INVALID_ID );
    KADEMLIA_TEST_ERROR( TRUNCATED_ID );
    KADEMLIA_TEST_ERROR( TRUNCATED_HEADER );
    KADEMLIA_TEST_ERROR( TRUNCATED_ENDPOINT );
    KADEMLIA_TEST_ERROR( TRUNCATED_ADDRESS );
    KADEMLIA_TEST_ERROR( TRUNCATED_SIZE );
    KADEMLIA_TEST_ERROR( UNKNOWN_PROTOCOL_VERSION );
    KADEMLIA_TEST_ERROR( CORRUPTED_BODY );
    KADEMLIA_TEST_ERROR( UNASSOCIATED_MESSAGE_ID );
    KADEMLIA_TEST_ERROR( INVALID_IPV4_ADDRESS );
    KADEMLIA_TEST_ERROR( INVALID_IPV6_ADDRESS );
    KADEMLIA_TEST_ERROR( UNIMPLEMENTED );
    KADEMLIA_TEST_ERROR( VALUE_NOT_FOUND );
    KADEMLIA_TEST_ERROR( TIMER_MALFUNCTION );
    KADEMLIA_TEST_ERROR( ALREADY_RUNNING );
}

BOOST_AUTO_TEST_CASE( error_category_is_kademlia )
{
    auto e = make_error_condition( k::UNKNOWN_ERROR );
    BOOST_REQUIRE_EQUAL( "kademlia", e.category().name() );
}

#undef KADEMLIA_TEST_ERROR

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

