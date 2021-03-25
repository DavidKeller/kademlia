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
#include "kademlia/message_serializer.hpp"
#include "kademlia/message.hpp"
#include "gtest/gtest.h"

namespace {

namespace k = kademlia;
namespace kd = k::detail;

struct MessageSerializerTest: public ::testing::Test
{
    MessageSerializerTest(): id_{"abcd"}
    {}

    kd::id id_;
protected:
    ~MessageSerializerTest() override
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};


TEST_F(MessageSerializerTest, can_be_constructed)
{
    kd::message_serializer s{ id_ };
    (void)s;
}

TEST_F(MessageSerializerTest, can_serialize_a_message_with_a_body)
{
    kd::message_serializer s{ id_ };
    kd::id const searched_id{ "1234" };
    kd::id const token{ "ABCD" };

    kd::find_peer_request_body const expected{ searched_id };
    auto const b = s.serialize(expected, token);

    auto i = std::begin(b), e = std::end(b);
    kd::header h;
    EXPECT_TRUE(! kd::deserialize(i, e, h));
    EXPECT_EQ(kd::header::V1, h.version_);
    EXPECT_EQ(kd::header::FIND_PEER_REQUEST, h.type_);
    EXPECT_EQ(id_, h.source_id_);
    EXPECT_EQ(token, h.random_token_);

    kd::find_peer_request_body actual;
    EXPECT_TRUE(! kd::deserialize(i, e, actual));
    EXPECT_TRUE(expected.peer_to_find_id_ == actual.peer_to_find_id_);

    EXPECT_TRUE(i == e);
}

TEST_F(MessageSerializerTest, can_serialize_a_message_without_body)
{
    kd::message_serializer s{ id_ };
    kd::id const searched_id{ "1234" };
    kd::id const token{ "ABCD" };

    auto const b = s.serialize(kd::header::PING_REQUEST, token);

    auto i = std::begin(b), e = std::end(b);
    kd::header h;
    EXPECT_TRUE(! kd::deserialize(i, e, h));
    EXPECT_EQ(kd::header::V1, h.version_);
    EXPECT_EQ(kd::header::PING_REQUEST, h.type_);
    EXPECT_EQ(id_, h.source_id_);
    EXPECT_EQ(token, h.random_token_);

    EXPECT_TRUE(i == e);
}

}

