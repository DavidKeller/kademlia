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

#ifndef KADEMLIA_TEST_HELPERS_TASK_FIXTURE_HPP
#define KADEMLIA_TEST_HELPERS_TASK_FIXTURE_HPP

#include <cstdint>
#include <system_error>

#include <boost/asio/io_service.hpp>

#include "kademlia/peer.hpp"
#include "common.hpp"
#include "tracker_mock.hpp"
#include "routing_table_mock.hpp"
#include "gtest/gtest.h"

namespace kademlia {
namespace test {


struct TaskFixture: public ::testing::Test
{
    TaskFixture
            ( void )
            : io_service_()
            , io_service_work_( io_service_ )
            , tracker_( io_service_ )
            , failure_()
            , routing_table_()
            , callback_call_count_()
    { }

    detail::peer
    create_peer
            ( std::string const& ip, detail::id const& id )
    {
        auto e = detail::to_ip_endpoint( ip, 5555 );

        return detail::peer{ id, e };
    }

    detail::peer
    create_and_add_peer
            ( std::string const& ip, detail::id const& id )
    {
        auto p = create_peer( ip, id );
        routing_table_.peers_.emplace_back( p.id_, p.endpoint_ );

        return p;
    }

    boost::asio::io_service io_service_;
    boost::asio::io_service::work io_service_work_;
    tracker_mock tracker_;
    std::error_code failure_;
    routing_table_mock routing_table_;
    std::size_t callback_call_count_;

protected:
    ~TaskFixture() override
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};


struct task_fixture

{
    task_fixture
        ( void )
        : io_service_()
        , io_service_work_( io_service_ )
        , tracker_( io_service_ )
        , failure_()
        , routing_table_()
        , callback_call_count_()
    { }

    detail::peer
    create_peer
        ( std::string const& ip, detail::id const& id )
    {
        auto e = detail::to_ip_endpoint( ip, 5555 );

        return detail::peer{ id, e };
    }

    detail::peer
    create_and_add_peer
        ( std::string const& ip, detail::id const& id )
    {
        auto p = create_peer( ip, id );
        routing_table_.peers_.emplace_back( p.id_, p.endpoint_ );

        return p;
    }

    boost::asio::io_service io_service_;
    boost::asio::io_service::work io_service_work_;
    tracker_mock tracker_;
    std::error_code failure_;
    routing_table_mock routing_table_;
    std::size_t callback_call_count_;
};

} // namespace test
} // namespace kademlia

#endif
