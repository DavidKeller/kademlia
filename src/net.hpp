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

#ifndef KADEMLIA_NET_HPP
#define KADEMLIA_NET_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <chrono>
#include <memory>
#include <boost/asio/io_service.hpp>

#include <kademlia/error.hpp>

#include "message_socket.hpp"
#include "message.hpp"
#include "message_serializer.hpp"
#include "subnet.hpp"
#include "response_dispatcher.hpp"
#include "timer.hpp"

namespace kademlia {
namespace detail {


/**
 *
 */
class net final
{
public:
    ///
    using on_new_request_type = std::function< void 
            ( message_socket::endpoint_type const&
            , header const&
            , buffer::const_iterator
            , buffer::const_iterator ) >;

public:
    /**
     *
     */
    net
        ( id const& my_id
        , on_new_request_type on_new_request
        , boost::asio::io_service & io_service
        , endpoint const& listen_on_ipv4
        , endpoint const& listen_on_ipv6 );

    /**
     *
     */
    void
    init
        ( void );
   
    /**
     *
     */
    template< typename Request, typename OnResponseReceived, typename OnError >
    void
    async_send_request
        ( id const& response_id
        , Request const& request
        , message_socket::endpoint_type const& e
        , timer::duration const& timeout
        , OnResponseReceived const& on_response_received
        , OnError const& on_error );

    /**
     *
     */
    template< typename Request >
    void
    async_send_request
        ( id const& response_id
        , Request const& request
        , message_socket::endpoint_type const& e );

    /**
     *
     */
    template< typename Response >
    void
    async_send_response
        ( id const& response_id
        , Response const& response
        , message_socket::endpoint_type const& e );

private:
    /**
     *
     */
    void
    schedule_receive_on_subnet
        ( subnet & current_subnet );

    /**
     *
     */
    static subnet
    create_ipv4_subnet
        ( boost::asio::io_service & io_service
        , endpoint const& ipv4_endpoint );

    /**
     *
     */
    static subnet
    create_ipv6_subnet
        ( boost::asio::io_service & io_service
        , endpoint const& ipv6_endpoint );

    /**
     *
     */
    subnet &
    get_subnet_for
        ( message_socket::endpoint_type const& e );

    /**
     *
     */
    template< typename OnResponseReceived, typename OnError >
    void
    register_temporary_association
        ( id const& response_id
        , timer::duration const& association_ttl
        , OnResponseReceived const& on_response_received
        , OnError const& on_error );

    /**
     *
     */
    void
    handle_new_message
        ( detail::message_socket::endpoint_type const& sender
        , detail::buffer const& message );

private:
    on_new_request_type on_new_request_;
    subnet ipv4_subnet_;
    subnet ipv6_subnet_;
    response_dispatcher response_dispatcher_;
    timer timer_;
    message_serializer message_serializer_;
};

template< typename Request, typename OnResponseReceived, typename OnError >
void
net::async_send_request
    ( id const& response_id
    , Request const& request
    , message_socket::endpoint_type const& e
    , timer::duration const& timeout
    , OnResponseReceived const& on_response_received
    , OnError const& on_error )
{ 
    // Generate the request buffer.
    auto message = message_serializer_.serialize( request, response_id );

    // This lamba will keep the request message alive.
    auto on_request_sent = [ this, response_id
                           , on_response_received, on_error
                           , timeout, message ] 
        ( std::error_code const& failure ) 
    {
        if ( failure )
            on_error( failure );
        else 
            register_temporary_association( response_id, timeout
                                          , on_response_received
                                          , on_error );
    };

    // Serialize the request and send it.
    get_subnet_for( e ).async_send( *message, e, on_request_sent );
}

template< typename Request >
void
net::async_send_request
    ( id const& response_id
    , Request const& request
    , message_socket::endpoint_type const& e )
{ async_send_response( response_id, request, e ); }

template< typename Response >
void
net::async_send_response
    ( id const& response_id
    , Response const& response
    , message_socket::endpoint_type const& e )
{ 
    auto message = message_serializer_.serialize( response, response_id );

    // This lamba will keep the response message alive.
    auto on_response_sent = [ message ] 
        ( std::error_code const& failure ) 
    { };

    // Serialize the message and send it.
    get_subnet_for( e ).async_send( *message, e, on_response_sent );
}

template< typename OnResponseReceived, typename OnError >
void
net::register_temporary_association
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

} // namespace detail
} // namespace kademlia

#endif // KADEMLIA_NET_HPP

