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

#include "helpers/common.hpp"

#include <vector>
#include <kademlia/error.hpp>

#include "message_dispatcher.hpp"

namespace k = kademlia;
namespace kd = k::detail;

/**
 */
BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( can_be_constructed_using_a_reactor )
{
    boost::asio::io_service io_service;
    BOOST_REQUIRE_NO_THROW( kd::message_dispatcher{ io_service } );
}

BOOST_AUTO_TEST_SUITE_END()

struct fixture : kd::task_base
{
    fixture()
        : io_service_{}
        , dispatcher_{ io_service_ }
        , timeouts_received_{}
        , messages_received_{}
    { }

    virtual std::error_code
    handle_message
        ( kd::header const& h
        , kd::buffer::const_iterator i
        , kd::buffer::const_iterator e )
        override
    { 
        messages_received_.push_back( h.random_token_ ); 
        return std::error_code{};
    }

    virtual void
    handle_message_timeout
        ( kd::id const& h )
        override
    { timeouts_received_.push_back( h ); } 
    
    virtual bool
    is_finished
        ( void )
        const
        override
    { return false; }

    boost::asio::io_service io_service_;
    kd::message_dispatcher dispatcher_;
    std::vector< kd::id > timeouts_received_;
    std::vector< kd::id > messages_received_;
};

/**
 */
BOOST_AUTO_TEST_SUITE( test_usage )

BOOST_FIXTURE_TEST_CASE( unknown_message_are_dropped , fixture )
{
    kd::header const h{ kd::header::V1, kd::header::PING_REQUEST };
    kd::buffer const b;
    auto result = dispatcher_.dispatch_message( h, b.begin(), b.end() );
    BOOST_REQUIRE( k::UNASSOCIATED_MESSAGE_ID == result );
}

BOOST_FIXTURE_TEST_CASE( known_messages_are_forwarded, fixture )
{
    kd::header const h1{ kd::header::V1, kd::header::PING_REQUEST 
                       , kd::id{}, kd::id{ "1" } };
    kd::header const h2{ kd::header::V1, kd::header::PING_REQUEST };
    kd::buffer const b;
    auto const infinite = kd::message_dispatcher::duration::max();

    BOOST_REQUIRE_EQUAL( 0, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( 0, timeouts_received_.size() );

    // Create the association.
    dispatcher_.associate_message_with_task_for( h1.random_token_
                                               , this
                                               , infinite );

    BOOST_REQUIRE_EQUAL( 0, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( 0, timeouts_received_.size() );

    // Send an unexpected message.
    auto result = dispatcher_.dispatch_message( h2, b.begin(), b.end() );
    BOOST_REQUIRE( k::UNASSOCIATED_MESSAGE_ID == result );
    BOOST_REQUIRE_EQUAL( 0, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( 0, timeouts_received_.size() );

    // Send the expected message.
    result = dispatcher_.dispatch_message( h1, b.begin(), b.end() );
    BOOST_REQUIRE( ! result );
    BOOST_REQUIRE_EQUAL( 1, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( h1.random_token_, messages_received_.front() );
    BOOST_REQUIRE_EQUAL( 0, timeouts_received_.size() );

    // Send the previously expected message again.
    result = dispatcher_.dispatch_message( h1, b.begin(), b.end() );
    BOOST_REQUIRE( k::UNASSOCIATED_MESSAGE_ID == result );
    BOOST_REQUIRE_EQUAL( 1, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( 0, timeouts_received_.size() );
}

BOOST_FIXTURE_TEST_CASE( associations_can_be_timeouted, fixture )
{
    kd::header const h{ kd::header::V1, kd::header::PING_REQUEST };
    kd::buffer const b;
    auto const immediate = kd::message_dispatcher::duration::zero();

    BOOST_REQUIRE_EQUAL( 0, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( 0, timeouts_received_.size() );
    // Create the association.
    dispatcher_.associate_message_with_task_for( h.random_token_
                                               , this
                                               , immediate );
    BOOST_REQUIRE_EQUAL( 1, io_service_.poll() );
    BOOST_REQUIRE_EQUAL( 0, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( 1, timeouts_received_.size() );
    BOOST_REQUIRE_EQUAL( h.random_token_, timeouts_received_.front() );

    // Send the expected message.
    auto result = dispatcher_.dispatch_message( h, b.begin(), b.end() );
    BOOST_REQUIRE( k::UNASSOCIATED_MESSAGE_ID == result );
    BOOST_REQUIRE_EQUAL( 0, messages_received_.size() );
    BOOST_REQUIRE_EQUAL( 1, timeouts_received_.size() );
}

BOOST_AUTO_TEST_SUITE_END()

