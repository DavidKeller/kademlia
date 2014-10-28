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

#ifndef KADEMLIA_TRACKER_HPP
#define KADEMLIA_TRACKER_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include "kademlia/log.hpp"
#include "kademlia/message_serializer.hpp"
#include "kademlia/response_router.hpp"
#include "kademlia/network.hpp"
#include "kademlia/message.hpp"
#include "kademlia/routing_table.hpp"
#include "kademlia/value_store.hpp"
#include "kademlia/constants.hpp"

namespace kademlia {
namespace detail {

/**
 *
 */
template< typename RandomEngineType, typename NetworkType >
class tracker final
{
public:
    ///
    using network_type = NetworkType;

    ///
    using endpoint_type = typename network_type::endpoint_type;

    ///
    using random_engine_type = RandomEngineType;

public:
    /**
     *
     */
    tracker
        ( boost::asio::io_service & io_service
        , id const& my_id
        , network_type & network
        , random_engine_type & random_engine )
            : response_router_( io_service )
            , message_serializer_( my_id )
            , network_( network )
            , random_engine_( random_engine )
    { }

    /**
     *
     */
    tracker
        ( tracker const& )
        = delete;

    /**
     *
     */
    tracker &
    operator=
        ( tracker const& )
        = delete;

    /**
     *
     */
    template< typename Request, typename OnResponseReceived, typename OnError >
    void
    send_request
        ( Request const& request
        , endpoint_type const& e
        , timer::duration const& timeout
        , OnResponseReceived const& on_response_received
        , OnError const& on_error )
    {
        id const response_id( random_engine_ );
        // Generate the request buffer.
        auto message = message_serializer_.serialize( request, response_id );

        // This lamba will keep the request message alive.
        auto on_request_sent = [ this, response_id
                               , on_response_received, on_error
                               , timeout ]
            ( std::error_code const& failure )
        {
            if ( failure )
                on_error( failure );
            else
                response_router_.register_temporary_callback( response_id, timeout
                                                            , on_response_received
                                                            , on_error );
        };

        // Serialize the request and send it.
        network_.send( message, e, on_request_sent );
    }

    /**
     *
     */
    template< typename Request >
    void
    send_request
        ( Request const& request
        , endpoint_type const& e )
    {
        id const response_id( random_engine_ );
        send_response( response_id, request, e );
    }

    /**
     *
     */
    template< typename Response >
    void
    send_response
        ( id const& response_id
        , Response const& response
        , endpoint_type const& e )
    {
        auto message = message_serializer_.serialize( response, response_id );

        auto on_response_sent = []
            ( std::error_code const& /* failure */ )
        { };

        network_.send( message, e, on_response_sent );
    }

    /**
     *
     */
    void
    handle_new_response
        ( endpoint_type const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    { response_router_.handle_new_response( s, h, i, e ); }

private:
    ///
    response_router response_router_;
    ///
    message_serializer message_serializer_;
    ///
    network_type & network_;
    ///
    random_engine_type & random_engine_;
};

} // namespace detail
} // namespace kademlia

#endif
