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

#include "corrupted_message.hpp"

#include <vector>

#include "kademlia/id.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/discover_neighbors_task.hpp"
#include "gtest/gtest.h"
#include "task_fixture.hpp"

namespace k = kademlia;
namespace kd = k::detail;

namespace {

using endpoints_type = std::vector< kd::ip_endpoint >;

struct DiscoverNeighborsTaskTest : k::test::TaskFixture
{
    void
    operator()(std::error_code const& failure)
    {
        ++ callback_call_count_;
        failure_ = failure;
    }
};

TEST_F(DiscoverNeighborsTaskTest, can_notify_error_when_initial_endpoints_fail_to_respond)
{
    kd::id const my_id{ "a" };

    // Assume initial peer resolves to 2 IPv4 addresses.
    endpoints_type const endpoints{ kd::to_ip_endpoint("192.168.1.2", 5555)
            , kd::to_ip_endpoint("192.168.1.3", 5555) };

    kd::start_discover_neighbors_task(my_id
            , tracker_
            , routing_table_
            , endpoints
            , std::ref(*this));

    io_service_.poll();

    // As neither of theses addresses responded, the task throws.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);
}

TEST_F(DiscoverNeighborsTaskTest, can_contact_endpoints_until_one_respond)
{
    kd::id const my_id{ "a" };

    // Assume initial peer resolves to 2 IPv4 addresses.
    auto const e1 = kd::to_ip_endpoint("192.168.1.2", 5555);
    auto const e2 = kd::to_ip_endpoint("192.168.1.3", 5555);
    auto const e3 = kd::to_ip_endpoint("::4", 5555);
    endpoints_type const endpoints{ e1, e2, e3 };

    auto p1 = create_peer("192.168.1.4", kd::id{ "b" });
    auto p2 = create_peer("192.168.1.5", kd::id{ "b" });
    auto p3 = create_peer("192.168.1.6", kd::id{ "b" });
    auto p4 = create_peer("192.168.1.7", kd::id{ "b" });
    kd::find_peer_response_body const req{ { p1, p2, p3, p4 } };
    tracker_.add_message_to_receive(e2, my_id, req);

    kd::start_discover_neighbors_task(my_id
            , tracker_
            , routing_table_
            , endpoints
            , std::ref(*this));

    io_service_.poll();

    kd::find_peer_request_body const fp{ my_id };
    // Task queried e3 but it didn't respond.
    EXPECT_TRUE(tracker_.has_sent_message(e3, fp));

    // Hence task queried e2 that responded without
    // providing new peer.
    EXPECT_TRUE(tracker_.has_sent_message(e2, fp));

    // Task didn't send any more message as previous peer responded.
    EXPECT_TRUE(! tracker_.has_sent_message());

    // the callback has been called.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(! failure_);

    // Ensure e2 response listed peer p1 has been added.
    EXPECT_EQ(req.peers_.size(), routing_table_.peers_.size());
}

TEST_F(DiscoverNeighborsTaskTest, can_skip_wrong_response)
{
    kd::id const my_id{ "a" };

    // Assume initial peer resolves to 2 IPv4 addresses.
    auto const e1 = kd::to_ip_endpoint("192.168.1.2", 5555);
    endpoints_type const endpoints{ e1 };

    kd::find_value_response_body const req{};
    tracker_.add_message_to_receive(e1, my_id, req);

    kd::start_discover_neighbors_task(my_id
            , tracker_
            , routing_table_
            , endpoints
            , std::ref(*this));

    io_service_.poll();

    // the callback has been called.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);
}

TEST_F(DiscoverNeighborsTaskTest, can_skip_corrupted_response)
{
    kd::id const my_id{ "a" };

    // Assume initial peer resolves to 2 IPv4 addresses.
    auto const e1 = kd::to_ip_endpoint("192.168.1.2", 5555);
    endpoints_type const endpoints{ e1 };

    k::test::corrupted_message< kd::header::FIND_PEER_RESPONSE > const req{};
    tracker_.add_message_to_receive(e1, my_id, req);

    kd::start_discover_neighbors_task(my_id
            , tracker_
            , routing_table_
            , endpoints
            , std::ref(*this));

    io_service_.poll();

    // the callback has been called.
    EXPECT_EQ(1, callback_call_count_);
    EXPECT_TRUE(failure_ == k::INITIAL_PEER_FAILED_TO_RESPOND);
}

}

