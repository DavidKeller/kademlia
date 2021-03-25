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


#include "routing_table_mock.hpp"
#include "task_fixture.hpp"
#include "kademlia/id.hpp"
#include "kademlia/notify_peer_task.hpp"
#include "gtest/gtest.h"

namespace {

namespace k = kademlia;
namespace kd = k::detail;

using NotifyPeerTaskTest = k::test::TaskFixture;


TEST_F(NotifyPeerTaskTest, can_query_known_peer_for_specific_id)
{
    kd::id const my_id{ "a" };
    routing_table_.expected_ids_.emplace_back(my_id);
    auto p1 = create_and_add_peer("192.168.1.2", kd::id{ "1a" });
    auto p2 = create_and_add_peer("192.168.1.3", kd::id{ "2a" });
    auto p3 = create_and_add_peer("192.168.1.4", kd::id{ "4a" });

    // p1 doesn't know closer peer.
    tracker_.add_message_to_receive(p1.endpoint_
                                   , p1.id_
                                   , kd::find_peer_response_body{});

    // p2 doesn't know closer peer.
    tracker_.add_message_to_receive(p2.endpoint_
                                   , p2.id_
                                   , kd::find_peer_response_body{});

    // p3 doesn't know closer peer.
    tracker_.add_message_to_receive(p3.endpoint_
                                   , p3.id_
                                   , kd::find_peer_response_body{});



    kd::start_notify_peer_task(my_id, tracker_, routing_table_);

    io_service_.poll();

    // Task queried routing table to find closest known peers.
    EXPECT_EQ(1, routing_table_.find_call_count_);

    // Task asked p1, p2 & p3 for a closer peer or the value.
    kd::find_peer_request_body const fv{ my_id };
    EXPECT_TRUE(tracker_.has_sent_message(p1.endpoint_, fv));
    EXPECT_TRUE(tracker_.has_sent_message(p2.endpoint_, fv));
    EXPECT_TRUE(tracker_.has_sent_message(p3.endpoint_, fv));

    // Task didn't send any more message.
    EXPECT_TRUE(! tracker_.has_sent_message());
}

}
