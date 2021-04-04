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

#include <memory>

#include <boost/asio/io_service.hpp>

#include "test_engine.hpp"

#include "common.hpp"

namespace {

namespace k = kademlia;
namespace d = k::detail;
namespace t = k::test;

template<typename ... InitialPeer >
std::unique_ptr< t::test_engine >
create_test_engine( boost::asio::io_service & io_service
                  , d::id const& id
                  , InitialPeer &&... initial_peer )
{
    k::endpoint ipv4_endpoint{ "127.0.0.1", k::session_base::DEFAULT_PORT };
    k::endpoint ipv6_endpoint{ "::1", k::session_base::DEFAULT_PORT };

    using engine_ptr = std::unique_ptr< t::test_engine >;

    engine_ptr t{ new t::test_engine{ io_service
                                    , std::forward< InitialPeer >( initial_peer )...
                                    , ipv4_endpoint, ipv6_endpoint
                                    , id } };
    return t;
}

BOOST_AUTO_TEST_SUITE( engine )

BOOST_AUTO_TEST_SUITE( test_usage )

BOOST_AUTO_TEST_CASE( isolated_engine_cannot_be_constructed )
{
    boost::asio::io_service io_service;

    k::endpoint initial_peer{ "172.18.1.2", k::session_base::DEFAULT_PORT };

    BOOST_REQUIRE_THROW( create_test_engine( io_service
                                           , d::id{}
                                           , initial_peer )
                       , std::exception );
}

BOOST_AUTO_TEST_CASE( two_engines_can_find_themselves )
{
    boost::asio::io_service io_service;

    d::id const id1{ "8000000000000000000000000000000000000000" };
    auto e1 = create_test_engine( io_service, id1 );

    BOOST_REQUIRE_EQUAL( 0, io_service.poll() );
  
    d::id const id2{ "4000000000000000000000000000000000000000" };
    auto e2 = create_test_engine( io_service, id2, e1->ipv4() );
}

BOOST_AUTO_TEST_CASE( two_engines_can_save_and_load )
{
    boost::asio::io_service io_service;

    d::id const id1{ "8000000000000000000000000000000000000000" };
    auto e1 = create_test_engine( io_service, id1 );

    d::id const id2{ "4000000000000000000000000000000000000000" };
    auto e2 = create_test_engine( io_service, id2, e1->ipv4() );

    std::string const expected_data{ "data" };

    auto on_save = [ &expected_data ]( std::error_code const& failure )
    { if ( failure ) throw std::system_error{ failure }; };
    e1->async_save( "key", expected_data, on_save );

    BOOST_REQUIRE_GT( io_service.poll(), 0 );

    auto on_load = [ &expected_data ]( std::error_code const& failure
                                     , std::string const& actual_data )
    {
        if ( failure ) throw std::system_error{ failure };
        if ( expected_data != actual_data )
            throw std::runtime_error{ "Unexpected data" };
    };
    e2->async_load( "key", on_load );

    BOOST_REQUIRE_GT( io_service.poll(), 0 );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

