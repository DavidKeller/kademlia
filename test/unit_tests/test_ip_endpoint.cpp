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
#include "kademlia/ip_endpoint.hpp"
#include "gtest/gtest.h"
#include <system_error>

namespace {

namespace k = kademlia;
namespace kd = k::detail;
namespace ba = boost::asio;


TEST(IPEndpoint, can_be_default_constructed)
{
    EXPECT_NO_THROW(
        kd::ip_endpoint const e{};
        (void)e;
   );
}

TEST(IPEndpoint, can_be_constructed_with_ip_and_port)
{
    EXPECT_NO_THROW(
        auto const e = kd::to_ip_endpoint( "192.168.0.1", 1234); 
        (void)e;
   );
}


TEST(IPEndpoint, can_be_compared)
{
    {
        auto a = kd::to_ip_endpoint( "192.168.0.1", 1234);
        auto b = a;
        EXPECT_EQ(a, b);
    }

    {
        auto a = kd::to_ip_endpoint( "192.168.0.1", 1234);
        auto b = kd::to_ip_endpoint( "192.168.0.2", 1234);
        EXPECT_NE(a, b);
    }
}


TEST(IPEndpoint, can_be_printed)
{
    boost::test_tools::output_test_stream out;
    out << kd::to_ip_endpoint( "192.168.0.1", 1234);
    EXPECT_TRUE( out.is_equal( "192.168.0.1:1234"));
}

}

