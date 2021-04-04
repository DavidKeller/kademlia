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

#include <cstdint>
#include <future>

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/system/system_error.hpp>

#include <kademlia/error.hpp>
#include <kademlia/session.hpp>
#include <kademlia/first_session.hpp>

#include "common.hpp"
#include "network.hpp"

namespace {

namespace k = kademlia;
namespace bo = boost::asio;

BOOST_AUTO_TEST_SUITE( session )

BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( session_opens_sockets_on_all_interfaces_by_default )
{
    k::first_session s;

    k::test::check_listening( "0.0.0.0", k::first_session::DEFAULT_PORT );
    k::test::check_listening( "::", k::first_session::DEFAULT_PORT );
}

BOOST_AUTO_TEST_CASE( session_opens_both_ipv4_ipv6_sockets )
{
    // Create listening socket.
    std::uint16_t const port1 = k::test::get_temporary_listening_port();
    std::uint16_t const port2 = k::test::get_temporary_listening_port( port1 );
    k::endpoint ipv4_endpoint{ "127.0.0.1", port1 };
    k::endpoint ipv6_endpoint{ "::1", port2 };

    k::first_session s{ ipv4_endpoint, ipv6_endpoint };

    k::test::check_listening( "127.0.0.1", port1 );
    k::test::check_listening( "::1", port2 );
}

BOOST_AUTO_TEST_CASE( session_throw_on_invalid_ipv6_address )
{
    // Create listening socket.
    std::uint16_t const port1 = k::test::get_temporary_listening_port();
    std::uint16_t const port2 = k::test::get_temporary_listening_port( port1 );
    k::endpoint ipv4_endpoint{ "127.0.0.1", port1 };
    k::endpoint ipv6_endpoint{ "0.0.0.0", port2 };

    BOOST_REQUIRE_THROW( k::first_session s( ipv4_endpoint, ipv6_endpoint )
                       , std::exception );
}

BOOST_AUTO_TEST_CASE( session_throw_on_invalid_ipv4_address )
{
    // Create listening socket.
    std::uint16_t const port1 = k::test::get_temporary_listening_port();
    std::uint16_t const port2 = k::test::get_temporary_listening_port( port1 );
    k::endpoint ipv4_endpoint{ "::", port1 };
    k::endpoint ipv6_endpoint{ "::1", port2 };

    BOOST_REQUIRE_THROW( k::first_session s( ipv4_endpoint, ipv6_endpoint )
                       , std::exception );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( test_usage )

BOOST_AUTO_TEST_CASE( session_run_can_be_aborted )
{
    k::first_session s{};

    auto result = std::async( std::launch::async
                            , &k::first_session::run, &s );
    s.abort();

    BOOST_REQUIRE( result.get() == k::RUN_ABORTED );
}

BOOST_AUTO_TEST_CASE( session_can_save_and_load )
{
    auto const fs_port = k::test::get_temporary_listening_port();
    k::endpoint const first_session_endpoint{ "127.0.0.1", fs_port };
    k::first_session fs{ first_session_endpoint
                       , k::endpoint{ "::1", fs_port } };

    auto fs_result = std::async( std::launch::async
                               , &k::first_session::run, &fs );

    auto const s_port = k::test::get_temporary_listening_port( fs_port );
    k::session s{ first_session_endpoint
                , k::endpoint{ "127.0.0.1", s_port }
                , k::endpoint{ "::1", s_port } };

    auto s_result = std::async( std::launch::async
                              , &k::session::run, &s );

    std::string const key{ "key" };
    std::string const expected_value{ "value" };

    std::string actual_value;
    auto on_load = [ &s, &actual_value ]
            ( std::error_code const& failure
            , k::session::data_type const& data )
    {
        if ( ! failure )
            actual_value.assign( data.begin(), data.end() ); 
        s.abort();
    };

    auto on_save = [ &s, &key, &on_load ]
            ( std::error_code const& failure )
    {
        if ( failure )
            s.abort();
        else
            s.async_load( key, on_load );
    };

    s.async_save( key, expected_value, on_save );

    BOOST_REQUIRE( s_result.get() == k::RUN_ABORTED );
    BOOST_REQUIRE_EQUAL( actual_value, expected_value );

    fs.abort();
    BOOST_REQUIRE( fs_result.get() == k::RUN_ABORTED );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

