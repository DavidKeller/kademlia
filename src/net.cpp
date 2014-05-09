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

#include "net.hpp"

namespace kademlia {
namespace detail {

net::net
    ( id const& my_id
    , on_new_request_type on_new_request
    , boost::asio::io_service & io_service
    , endpoint const& listen_on_ipv4
    , endpoint const& listen_on_ipv6 )
        : on_new_request_{ on_new_request }
        , ipv4_subnet_{ create_ipv4_subnet( io_service, listen_on_ipv4 ) }
        , ipv6_subnet_{ create_ipv6_subnet( io_service, listen_on_ipv6 ) }
        , response_dispatcher_{}
        , timer_{ io_service }
        , message_serializer_{ my_id }
{ }
    
void
net::init
    ( void )
{
    schedule_receive_on_subnet( ipv4_subnet_ );
    schedule_receive_on_subnet( ipv6_subnet_ );
}

void
net::schedule_receive_on_subnet
    ( subnet & current_subnet )
{
    auto on_new_message = [ this, &current_subnet ]
        ( std::error_code const& failure
        , message_socket::endpoint_type const& sender
        , buffer const& message )
    {
        if ( ! failure )
            handle_new_message( sender, message );

        schedule_receive_on_subnet( current_subnet );
    };

    current_subnet.async_receive( on_new_message );
}

subnet
net::create_ipv4_subnet
    ( boost::asio::io_service & io_service
    , endpoint const& ipv4_endpoint )
{
    auto endpoints = resolve_endpoint( io_service, ipv4_endpoint );

    for ( auto const& i : endpoints )
    {
        if ( i.address().is_v4() )
            return subnet{ create_socket( io_service, i ) };
    }

    throw std::system_error{ make_error_code( INVALID_IPV4_ADDRESS ) };
}

subnet
net::create_ipv6_subnet
    ( boost::asio::io_service & io_service
    , endpoint const& ipv6_endpoint )
{
    auto endpoints = resolve_endpoint( io_service, ipv6_endpoint );

    for ( auto const& i : endpoints )
    {
        if ( i.address().is_v6() )
            return subnet{ create_socket( io_service, i ) };
    }

    throw std::system_error{ make_error_code( INVALID_IPV6_ADDRESS ) };
}

subnet &
net::get_subnet_for
    ( message_socket::endpoint_type const& e )
{
    if ( e.address().is_v4() )
        return ipv4_subnet_;

    return ipv6_subnet_;
}

void
net::handle_new_message
    ( detail::message_socket::endpoint_type const& sender
    , detail::buffer const& message )
{ 
    auto i = message.begin(), e = message.end();

    detail::header h;
    // Try to deserialize header.
    if ( deserialize( i, e, h ) )
        return;

    // Try to forward the message to its associated callback.
    auto failure = response_dispatcher_.dispatch_message( sender
                                                        , h, i, e ); 

    // If no callback was associated, forward the message
    // to the request handler.
    if ( failure == UNASSOCIATED_MESSAGE_ID )
        on_new_request_( sender, h, i, e );
}

} // namespace detail
} // namespace kademlia

