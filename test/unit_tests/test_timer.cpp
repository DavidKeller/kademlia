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
#include "kademlia/error_impl.hpp"
#include "kademlia/timer.hpp"
#include "gtest/gtest.h"
#include <vector>

namespace {

namespace k = kademlia;
namespace kd = k::detail;


TEST(TimerConstructTest, can_be_constructed_using_a_reactor)
{
    boost::asio::io_service io_service;
    EXPECT_NO_THROW(kd::timer{ io_service });
}


struct TimerTest: public ::testing::Test
{
    TimerTest()
        : io_service_{}
        , work_{ io_service_ }
        , manager_{ io_service_ }
        , timeouts_received_{}
    { }

    boost::asio::io_service io_service_;
    boost::asio::io_service::work work_;
    kd::timer manager_;
    std::size_t timeouts_received_;

protected:
    ~TimerTest() override
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};


TEST_F(TimerTest, multiple_associations_can_be_added)
{
    EXPECT_EQ(0, io_service_.poll());
    EXPECT_EQ(0, timeouts_received_);

    // Create the association.
    auto on_expiration = [ this ] (void)
    { ++ timeouts_received_; };

    auto const infinite = std::chrono::hours(1);
    manager_.expires_from_now(infinite, on_expiration);
    EXPECT_EQ(0, io_service_.poll());
    EXPECT_EQ(0, timeouts_received_);

    // This new expiration should trigger a cancel of the current
    // timeout (infinite), hence one task execution.
    auto const immediate = kd::timer::duration::zero();
    manager_.expires_from_now(immediate, on_expiration);
    EXPECT_EQ(1, io_service_.run_one());
    EXPECT_EQ(0, timeouts_received_);

    // Then the task execution of the new timeout (immediate)
    // and the call of its associated callback.
    EXPECT_EQ(1, io_service_.run_one());
    EXPECT_EQ(1, timeouts_received_);

    EXPECT_EQ(0, io_service_.poll());
    EXPECT_EQ(1, timeouts_received_);

    // A timeout (infinite) is still in flight atm.
}


}
