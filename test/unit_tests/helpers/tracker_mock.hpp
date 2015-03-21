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

#include <vector>
#include <queue>

#include <kademlia/error.hpp>

#include "kademlia/message.hpp"

namespace kademlia {
namespace tests {

struct tracker_mock
{
    struct sent_message
    {
        detail::header::type type;
        detail::ip_endpoint endpoint;
    };

    struct message_to_receive
    {
        detail::ip_endpoint endpoint;
        detail::header header;
        detail::buffer body;
    };

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
        auto id = detail::message_traits< RequestType >::TYPE_ID;
        sent_requests_.push_back( sent_message{ id, endpoint } );

        if (responses_to_receive_.empty())
            on_error(make_error_code(UNIMPLEMENTED));
        else {
            auto & response = responses_to_receive_.front();
            on_message_received(response.endpoint,
                                response.header,
                                response.body.begin(),
                                response.body.end());
            responses_to_receive_.pop();
        }
    }

    template< typename RequestType
            , typename EndpointType >
    void
    send_request
        ( RequestType const& request
        , EndpointType const& e )
    {
        auto id = detail::message_traits< RequestType >::TYPE_ID;
        sent_requests_.push_back( sent_message{ id, e } );
    }

    template< typename ResponseType
            , typename EndpointType >
    void
    send_response
        ( detail::id const& response_id
        , ResponseType const& response
        , EndpointType const& e )
    {
        auto id = detail::message_traits< ResponseType >::TYPE_ID;
        sent_responses_.push_back( sent_message{ id, e } );
    }

    std::queue< message_to_receive > responses_to_receive_;
    std::vector< sent_message > sent_requests_;
    std::vector< sent_message > sent_responses_;
};

} // namespace tests
} // namespace kademlia

#endif // KADEMLIA_TEST_HELPERS_TRACKER_MOCK_HPP

