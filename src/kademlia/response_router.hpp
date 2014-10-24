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

#ifndef KADEMLIA_RESPONSE_ROUTER_HPP
#define KADEMLIA_RESPONSE_ROUTER_HPP

#include "kademlia/ip_endpoint.hpp"
#include "kademlia/response_dispatcher.hpp"
#include "kademlia/timer.hpp"

#ifdef _MSC_VER
#   pragma once
#endif

namespace kademlia {
namespace detail {

/**
 *
 */
class response_router final
{
public:
    ///
    using endpoint_type = ip_endpoint;

public:
    /**
     *
     */
    explicit
    response_router
        ( boost::asio::io_service & io_service )
            : response_dispatcher_()
            , timer_( io_service )
    { }

    /**
     *
     */
    response_router
        ( response_router const& )
        = delete;

    /**
     *
     */
    response_router &
    operator=
        ( response_router const& )
        = delete;

    /**
     *
     */
    void
    handle_new_response
        ( endpoint_type const& sender
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    {
        // Try to forward the message to its associated callback.
        auto failure = response_dispatcher_.dispatch_message( sender
                                                            , h, i, e );
        if ( failure == UNASSOCIATED_MESSAGE_ID )
            // Unknown request or unassociated responses
            // are discarded.
            LOG_DEBUG( response_router, this ) << "dropping unknown response."
                    << std::endl;
    }

    /**
     *
     */
    template< typename OnResponseReceived, typename OnError >
    void
    register_temporary_association
        ( id const& response_id
        , timer::duration const& association_ttl
        , OnResponseReceived const& on_response_received
        , OnError const& on_error )
    {
        auto on_timeout = [ this, on_error, response_id ]
            ( void )
        {
            // If an association has been removed, that means
            // the message has never been received
            // hence report the timeout to the client.
            if ( response_dispatcher_.remove_association( response_id ) )
                on_error( make_error_code( std::errc::timed_out ) );
        };

        // Associate the response id with the
        // on_response_received callback.
        response_dispatcher_.push_association( response_id
                                             , on_response_received );

        timer_.expires_from_now( association_ttl, on_timeout );
    }

private:
    ///
    response_dispatcher response_dispatcher_;
    ///
    timer timer_;
};

} // namespace detail
} // namespace kademlia

#endif
