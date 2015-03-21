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

#ifndef KADEMLIA_TEST_HELPERS_TRACKER_MOCK_HPP
#define KADEMLIA_TEST_HELPERS_TRACKER_MOCK_HPP

#include <queue>

#include <kademlia/error.hpp>

#include "kademlia/message.hpp"
#include "kademlia/message_serializer.hpp"

namespace kademlia {
namespace tests {

class tracker_mock
{
public:
    /**
     *
     */
    tracker_mock
        ( void )
            : message_serializer_( detail::id{} )
            , responses_to_receive_()
            , sent_messages_()
    { }

    /**
     *
     */
    template< typename MessageType >
    void
    add_message_to_receive
        ( detail::ip_endpoint const& endpoint
        , detail::header const& header
        , MessageType const& message )
    {
        message_to_receive m{ endpoint
                            , header };
        detail::serialize( message, m.body );

        responses_to_receive_.push( std::move( m ) );
    }

    /**
     *
     */
    template< typename MessageType >
    bool
    has_sent_message
        ( detail::ip_endpoint const& endpoint
        , detail::header const& header
        , MessageType const& message )
    {
        if ( sent_messages_.empty() )
            return false;

        auto const c = sent_messages_.front();
        sent_messages_.pop();

        sent_message const e{ endpoint
                            , message_serializer_.serialize( message
                                                           , detail::id{} ) };

        return c.endpoint == e.endpoint && c.message == e.message;
    }

    /**
     *
     */
    bool
    has_sent_message
        ( void )
    { return ! sent_messages_.empty(); }

    /**
     *
     */
    template< typename RequestType
            , typename EndpointType
            , typename TimeoutType
            , typename OnMessageReceiveCallback
            , typename OnErrorCallback >
    void
    send_request
        ( RequestType const& request
        , EndpointType const& endpoint
        , TimeoutType const& timeout
        , OnMessageReceiveCallback const& on_message_received
        , OnErrorCallback const& on_error )
    {
        save_sent_message( request, endpoint );

        if ( responses_to_receive_.empty() )
            on_error( make_error_code( UNIMPLEMENTED ) );
        else {
            auto & response = responses_to_receive_.front();
            on_message_received( response.endpoint,
                                 response.header,
                                 response.body.begin(),
                                 response.body.end() );
            responses_to_receive_.pop();
        }
    }

    /**
     *
     */
    template< typename RequestType
            , typename EndpointType >
    void
    send_request
        ( RequestType const& r
        , EndpointType const& e )
    { save_sent_message( r, e ); }

    /**
     *
     */
    template< typename ResponseType
            , typename EndpointType >
    void
    send_response
        ( detail::id const&
        , ResponseType const& r
        , EndpointType const& e )
    { save_sent_message( r, e ); }

private:
    struct sent_message
    {
        detail::ip_endpoint endpoint;
        detail::buffer message;
    };

    struct message_to_receive
    {
        detail::ip_endpoint endpoint;
        detail::header header;
        detail::buffer body;
    };

private:
    /**
     *
     */
    template< typename RequestType
            , typename EndpointType >
    void
    save_sent_message
        ( RequestType const& request
        , EndpointType const& endpoint )
    {
        sent_message m{ endpoint
                      , message_serializer_.serialize( request
                                                     , detail::id{} ) };
        sent_messages_.push( m );
    }

private:
    ///
    detail::message_serializer message_serializer_;
    ///
    std::queue< message_to_receive > responses_to_receive_;
    ///
    std::queue< sent_message > sent_messages_;
};

} // namespace tests
} // namespace kademlia

#endif // KADEMLIA_TEST_HELPERS_TRACKER_MOCK_HPP

