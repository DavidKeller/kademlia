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

#ifndef KADEMLIA_NETWORK_HPP
#define KADEMLIA_NETWORK_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <functional>
#include <boost/asio/io_service.hpp>

#include "log.hpp"
#include "ip_endpoint.hpp"
#include "message_socket.hpp"
#include "buffer.hpp"

namespace kademlia {
namespace detail {

/**
 *
 */
template< typename MessageSocketType >
class network final
{
public:
    ///
    using message_socket_type = MessageSocketType;

    ///
    using endpoint_type = ip_endpoint;

    ///
    using resolved_endpoints = std::vector< endpoint_type >;

    ///
    using on_message_received_type = std::function<
        void ( endpoint_type const&
             , buffer::const_iterator
             , buffer::const_iterator ) >;
public:
    /**
     *
     */
    network
        ( boost::asio::io_service & io_service
        , message_socket_type && socket_ipv4
        , message_socket_type && socket_ipv6
        , on_message_received_type on_message_received )
            : io_service_( io_service )
            , socket_ipv4_( std::move( socket_ipv4 ) )
            , socket_ipv6_( std::move( socket_ipv6 ) )
            , on_message_received_( on_message_received )
    {
        start_message_reception();
        LOG_DEBUG( network, this ) << "created at '"
                << socket_ipv4_.local_endpoint() << "' and '"
                << socket_ipv6_.local_endpoint() << "'." << std::endl;
    }

    /**
     *
     */
    network
        ( network const& )
        = delete;

    /**
     *
     */
    network &
    operator=
        ( network const& )
        = delete;

    /**
     *
     */
    template< typename Message, typename OnMessageSent >
    void
    send
        ( Message const& message
        , endpoint_type const& e
        , OnMessageSent const& on_message_sent )
    { get_socket_for( e ).async_send( message, e, on_message_sent ); }

    /**
     *
     */
    template< typename Endpoint >
    resolved_endpoints
    resolve_endpoint
        ( Endpoint const& e )
    { return message_socket_type::resolve_endpoint( io_service_, e ); }

private:
    /**
     *
     */
    void
    start_message_reception
        ( void )
    {
        schedule_receive_on_socket( socket_ipv4_ );
        schedule_receive_on_socket( socket_ipv6_ );
    }

    /**
     *
     */
    void
    schedule_receive_on_socket
        ( message_socket_type & current_subnet )
    {
        auto on_new_message = [ this, &current_subnet ]
            ( std::error_code const& failure
            , endpoint_type const& sender
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            // Reception failure are fatal.
            if ( failure )
                throw std::system_error{ failure };

            on_message_received_( sender, i, e );
            schedule_receive_on_socket( current_subnet );
        };

        current_subnet.async_receive( on_new_message );
    }

    /**
     *
     */
    message_socket_type &
    get_socket_for
        ( endpoint_type const& e )
    {
        if ( e.address_.is_v4() )
            return socket_ipv4_;

        return socket_ipv6_;
    }

private:
    ///
    boost::asio::io_service & io_service_;
    ///
    message_socket_type socket_ipv4_;
    ///
    message_socket_type socket_ipv6_;
    ///
    on_message_received_type on_message_received_;
};

} // namespace detail
} // namespace kademlia

#endif
