// Copyright (c) 2013, David Keller
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

#include <utility>

#include "helpers/common.hpp"
#include "helpers/peer_factory.hpp"

namespace k = kademlia;
namespace kd = k::detail;

/**
 *  Test peer::peer()
 */
BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( cannot_be_instantiated_without_endpoint )
{
    kd::peer::endpoints_type peer_endpoints;
    peer_endpoints.push_back( k::endpoint( "localhost", "1" ) );
        
    BOOST_REQUIRE_NO_THROW( kd::peer new_peer( peer_endpoints ) );
}

BOOST_AUTO_TEST_SUITE_END()


/**
 *  Test peer::operator==()
 */
BOOST_AUTO_TEST_SUITE( test_comparison )

BOOST_AUTO_TEST_CASE( peers_endpoints_are_checked_during_comparison )
{
    kd::peer p1 = create_peer( "localhost", "1" );
    kd::peer p1bis = create_peer( "localhost", "1" );
    kd::peer p2 = create_peer( "localhost", "2" );

    BOOST_CHECK_EQUAL( p1, p1bis );
    BOOST_CHECK_NE( p1, p2 );
}


BOOST_AUTO_TEST_SUITE_END()

/**
 *  Test operator<<()
 */
BOOST_AUTO_TEST_SUITE( test_print )

BOOST_AUTO_TEST_CASE( peer_is_printable )
{
    std::ostringstream out;
    out << create_peer();
    BOOST_REQUIRE_NE( out.str().size(), 0 );
}

BOOST_AUTO_TEST_SUITE_END()
