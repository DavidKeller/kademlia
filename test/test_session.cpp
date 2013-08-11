// Copyright (c) 2010, David Keller
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

#include <boost/cstdint.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/system_error.hpp>
#include <kademlia/session.hpp>

#include "helpers/common.hpp"

namespace k = kademlia;
namespace bo = boost::asio;
using bo::ip::udp;
using bo::ip::tcp;

namespace {

template< typename Socket >
boost::system::error_code
create_socket
    ( std::string const& ip
    , boost::uint16_t port )
{
    auto const a = bo::ip::address::from_string( ip );
    bo::io_service io_service;

    // Try to create a socket.
    typename Socket::endpoint_type endpoint( a, port );
    Socket socket( io_service, endpoint.protocol() );

    boost::system::error_code failure;
    socket.bind( endpoint, failure );

    return failure;
}

void
check_listening
    ( std::string const& ip
    , boost::uint16_t port )
{
    auto udp_failure = create_socket< udp::socket >( ip, port ); 
    BOOST_REQUIRE_EQUAL( boost::system::errc::address_in_use, udp_failure );

    auto tcp_failure = create_socket< tcp::acceptor >( ip, port );
    BOOST_REQUIRE_EQUAL( boost::system::errc::address_in_use, tcp_failure );
}

} // namespace

/**
 *  Test routing_table::routing_table()
 */
BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( session_opens_sockets )
{
    // Create listening socket.
    std::vector< k::endpoint > es;
    es.push_back( k::endpoint( "127.0.0.1", "11111" ) );
    es.push_back( k::endpoint( "127.0.0.1", "11112" ) );
    
    // Create Dummy initial peer.
    k::endpoint const initial_peer( "127.0.0.1", "22222" );
    
    k::session s( es, initial_peer );
    
    check_listening( "127.0.0.1", 11111 );
    check_listening( "127.0.0.1", 11112 );
}

BOOST_AUTO_TEST_SUITE_END()
