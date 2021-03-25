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


#include "tracker_mock.hpp"
#include "corrupted_message.hpp"
#include "task_fixture.hpp"
#include "kademlia/id.hpp"
#include "kademlia/peer.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/store_value_task.hpp"
#include "gtest/gtest.h"
#include <vector>


namespace {

namespace k = kademlia;
namespace kd = k::detail;

using data_type = std::vector< std::uint8_t >;

struct StoreValueTaskTest : k::test::TaskFixture
{
    void operator()(std::error_code const& f)
    {
        ++ callback_call_count_;
        failure_ = f;
    }
};


TEST_F(StoreValueTaskTest, can_notify_error_when_routing_table_is_empty)
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back(chosen_key);

    EXPECT_EQ(0, routing_table_.find_call_count_);

    kd::start_store_value_task< data_type >(chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref(*this));

    io_service_.poll();

    // Task queried routing table to find closest known peers.
    EXPECT_EQ(1, routing_table_.find_call_count_);

    // Task didn't send any more message.
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);

    // Task notified the error.
    EXPECT_TRUE(! tracker_.has_sent_message());
    EXPECT_EQ(1, callback_call_count_);
}

TEST_F(StoreValueTaskTest, can_notify_error_when_unique_peer_fails_to_respond)
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back(chosen_key);

    auto p1 = create_and_add_peer("192.168.1.1", kd::id{ "b" });

    kd::start_store_value_task< data_type >(chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref(*this));
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    EXPECT_EQ(1, routing_table_.find_call_count_);

    // Task asked p1 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    EXPECT_TRUE(tracker_.has_sent_message(p1.endpoint_, fv));

    // Task didn't send any more message.
    EXPECT_TRUE(! tracker_.has_sent_message());

    // Task notified the error.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);
}

TEST_F(StoreValueTaskTest, can_notify_error_when_all_peers_fail_to_respond)
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back(chosen_key);

    auto p1 = create_and_add_peer("192.168.1.1", kd::id{ "b" });
    auto p2 = create_and_add_peer("192.168.1.2", kd::id{ "c" });

    kd::start_store_value_task< data_type >(chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref(*this));
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    EXPECT_EQ(1, routing_table_.find_call_count_);

    // Task asked p1 & p2 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    EXPECT_TRUE(tracker_.has_sent_message(p1.endpoint_, fv));
    EXPECT_TRUE(tracker_.has_sent_message(p2.endpoint_, fv));

    // Task didn't send any more message.
    EXPECT_TRUE(! tracker_.has_sent_message());

    // Task notified the error.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);
}

TEST_F(StoreValueTaskTest, can_store_value_when_already_known_peer_is_the_target)
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back(chosen_key);

    auto p1 = create_and_add_peer("192.168.1.1", kd::id{ "b" });
    kd::find_peer_response_body const b1{};
    tracker_.add_message_to_receive(p1.endpoint_, p1.id_, b1);
    kd::start_store_value_task< data_type >(chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref(*this));
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    EXPECT_EQ(1, routing_table_.find_call_count_);

    // Task asked p1 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    EXPECT_TRUE(tracker_.has_sent_message(p1.endpoint_, fv));

    // Task decided that p1 was the closest
    // hence it asked to store data on it.
    kd::store_value_request_body const sv{ chosen_key, data };
    EXPECT_TRUE(tracker_.has_sent_message(p1.endpoint_, sv));

    // Task didn't send any more message.
    EXPECT_TRUE(! tracker_.has_sent_message());

    // Task notified the success.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(! failure_);
}

TEST_F(StoreValueTaskTest, can_store_value_when_discovered_peer_is_the_target)
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back(chosen_key);

    // p1 is the only known peer atm.
    auto p1 = create_and_add_peer("192.168.1.1", kd::id{ "b" });

    // p2 is unknown atm.
    auto i2 = kd::id{ chosen_key };
    auto e2 = kd::to_ip_endpoint("192.168.1.2", 5555);
    kd::peer const p2{ i2, e2 };

    // p1 knows p2.
    kd::find_peer_response_body const fp1{ { p2 } };
    tracker_.add_message_to_receive(p1.endpoint_, p1.id_, fp1);

    // And p2 doesn't know closer peer.
    kd::find_peer_response_body const fp2{};
    tracker_.add_message_to_receive(e2, i2, fp2);

    kd::start_store_value_task< data_type >(chosen_key
                                           , data
                                           , tracker_
                                           , routing_table_
                                           , std::ref(*this));
    io_service_.poll();

    // Task queried routing table to find closest known peers.
    EXPECT_EQ(1, routing_table_.find_call_count_);

    // Task asked p1 for a closer peer.
    kd::find_peer_request_body const fv{ chosen_key };
    EXPECT_TRUE(tracker_.has_sent_message(p1.endpoint_, fv));

    // Task then asked p2 for a closer peer.
    EXPECT_TRUE(tracker_.has_sent_message(e2, fv));

    // Task decided that p2 was the closest
    // hence it asked to store data on it.
    kd::store_value_request_body const sv{ chosen_key, data };
    EXPECT_TRUE(tracker_.has_sent_message(e2, sv));

    // Task is also required to store data 
    // on close peers for redundancy purpose.
    EXPECT_TRUE(tracker_.has_sent_message(p1.endpoint_, sv));

    // Task didn't send any more message.
    EXPECT_TRUE(! tracker_.has_sent_message());

    // Task notified the success.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(! failure_);
}

TEST_F(StoreValueTaskTest, can_skip_wrong_response)
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back(chosen_key);

    // p1 is the only known peer.
    auto p1 = create_and_add_peer("192.168.1.1", kd::id{ "b" });

    kd::find_value_response_body const req{};
    tracker_.add_message_to_receive(p1.endpoint_, p1.id_, req);

    kd::start_store_value_task(chosen_key
                              , data
                              , tracker_
                              , routing_table_
                              , std::ref(*this));

    io_service_.poll();

    // the callback has been called.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);
}

TEST_F(StoreValueTaskTest, can_skip_corrupted_response)
{
    kd::id const chosen_key{ "a" };
    kd::buffer const data{ 1, 2, 3, 4 };
    routing_table_.expected_ids_.emplace_back(chosen_key);

    // p1 is the only known peer.
    auto p1 = create_and_add_peer("192.168.1.1", kd::id{ "b" });

    k::test::corrupted_message< kd::header::FIND_PEER_RESPONSE > const req{};
    tracker_.add_message_to_receive(p1.endpoint_, p1.id_, req);

    kd::start_store_value_task(chosen_key
                              , data
                              , tracker_
                              , routing_table_
                              , std::ref(*this));

    io_service_.poll();

    // the callback has been called.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);
}


}
