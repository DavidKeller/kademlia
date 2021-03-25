
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
#include "kademlia/peer.hpp"
#include "gtest/gtest.h"
#include <sstream>


namespace {

namespace kd = kademlia::detail;

struct PeerTest: public ::testing::Test
{
    PeerTest()
            : id_{}
            , ip_endpoint_(kd::to_ip_endpoint("127.0.0.1", 1234))
    { }

    kd::id id_;
    kd::ip_endpoint ip_endpoint_;

protected:
    ~PeerTest() override
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};


TEST_F(PeerTest, can_be_constructed)
{
    kd::peer const p{ id_, ip_endpoint_ };
    (void)p;
}

TEST_F(PeerTest, can_be_printed)
{
    boost::test_tools::output_test_stream out;

    out << kd::peer{ id_, ip_endpoint_ };

    std::ostringstream expected;
    expected << id_ << "@" << ip_endpoint_;
    EXPECT_TRUE(out.is_equal(expected.str()));
}


}

