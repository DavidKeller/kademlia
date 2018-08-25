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

#include <system_error>

#include "kademlia/ip_endpoint.hpp"

namespace {

namespace k = kademlia;
namespace kd = k::detail;
namespace ba = boost::asio;

BOOST_AUTO_TEST_SUITE( ip_endpoint )

BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( can_be_default_constructed )
{
    BOOST_REQUIRE_NO_THROW(
        kd::ip_endpoint const e{};
        (void)e;
    );
}

BOOST_AUTO_TEST_CASE( can_be_constructed_with_ip_and_port )
{
    BOOST_REQUIRE_NO_THROW(
        auto const e = kd::to_ip_endpoint( "192.168.0.1", 1234 ); 
        (void)e;
    );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( test_usage )

BOOST_AUTO_TEST_CASE( can_be_compared)
{
    {
        auto a = kd::to_ip_endpoint( "192.168.0.1", 1234 );
        auto b = a;

        BOOST_REQUIRE_EQUAL( a, b );
    }

    {
        auto a = kd::to_ip_endpoint( "192.168.0.1", 1234 );
        auto b = kd::to_ip_endpoint( "192.168.0.2", 1234 );

        BOOST_REQUIRE_NE( a, b );
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( test_print )

BOOST_AUTO_TEST_CASE( can_be_printed )
{
    boost::test_tools::output_test_stream out;

    out << kd::to_ip_endpoint( "192.168.0.1", 1234 );

    BOOST_CHECK( out.is_equal( "192.168.0.1:1234" ) );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

