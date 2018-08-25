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

#include <boost/asio/ip/udp.hpp>

#include <kademlia/endpoint.hpp>

#include "kademlia/ip_endpoint.hpp"
#include "kademlia/message_socket.hpp"

#include "common.hpp"
#include "network.hpp"

namespace {

namespace k = kademlia;
namespace kd = kademlia::detail;

using message_socket_type = kd::message_socket< boost::asio::ip::udp::socket >;

BOOST_AUTO_TEST_SUITE( message_socket )

BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( faulty_address_are_detected )
{
    boost::asio::io_service io_service;

    {
        k::endpoint const endpoint{ "error"
                                  , "27980" };

        BOOST_REQUIRE_THROW(
            message_socket_type::resolve_endpoint( io_service, endpoint );
        , std::exception );
    }
}

BOOST_AUTO_TEST_CASE( dns_can_be_resolved )
{
    boost::asio::io_service io_service;

    k::endpoint const endpoint{ "localhost"
                              , "27980" };

    auto const e = message_socket_type::resolve_endpoint( io_service, endpoint );

    BOOST_REQUIRE_LE( 1, e.size() );
}

BOOST_AUTO_TEST_CASE( ipv4_address_can_be_resolved )
{
    boost::asio::io_service io_service;

    k::endpoint const endpoint{ "127.0.0.1"
                              , "27980" };

    auto const e = message_socket_type::resolve_endpoint( io_service, endpoint );

    BOOST_REQUIRE_EQUAL( 1, e.size() );
}

BOOST_AUTO_TEST_CASE( ipv6_address_can_be_resolved )
{
    boost::asio::io_service io_service;

    k::endpoint const endpoint{ "::1"
                              , "27980" };

    auto e = message_socket_type::resolve_endpoint( io_service, endpoint );
    BOOST_REQUIRE_EQUAL( 1, e.size() );
}


BOOST_AUTO_TEST_CASE( ipv4_socket_can_be_created )
{
    boost::asio::io_service io_service;

    k::endpoint const endpoint( "127.0.0.1"
                              , k::test::get_temporary_listening_port() );

    BOOST_REQUIRE_NO_THROW(
        message_socket_type::ipv4( io_service, endpoint );
    );
}

BOOST_AUTO_TEST_CASE( ipv6_socket_can_be_created )
{
    boost::asio::io_service io_service;

    k::endpoint const endpoint( "::1"
                              , k::test::get_temporary_listening_port() );

    BOOST_REQUIRE_NO_THROW(
        message_socket_type::ipv6( io_service, endpoint );
    );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

