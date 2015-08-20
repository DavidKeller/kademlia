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

#ifndef KADEMLIA_MESSAGE_SOCKET_HPP
#define KADEMLIA_MESSAGE_SOCKET_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <vector>
#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/asio/buffer.hpp>

#include "kademlia/error_impl.hpp"
#include <kademlia/detail/cxx11_macros.hpp>

#include "kademlia/buffer.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/boost_to_std_error.hpp"

namespace kademlia {
namespace detail {

/**
 *
 */
template< typename UnderlyingSocketType >
class message_socket final
{
public:
    /// Consider we won't receive IPv6 jumbo datagram.
    static CXX11_CONSTEXPR std::size_t INPUT_BUFFER_SIZE = UINT16_MAX;

    ///
    using endpoint_type = ip_endpoint;

    ///
    using resolved_endpoints = std::vector< endpoint_type >;

public:
    /**
     *
     */
    template< typename EndpointType >
    static resolved_endpoints
    resolve_endpoint
        ( boost::asio::io_service & io_service
        , EndpointType const& e );

    /**
     *
     */
    template< typename EndpointType >
    static message_socket
    ipv4
        ( boost::asio::io_service & io_service
        , EndpointType const& e );

    /**
     *
     */
    template< typename EndpointType >
    static message_socket
    ipv6
        ( boost::asio::io_service & io_service
        , EndpointType const& e );

    /**
     *
     */
    message_socket
        ( message_socket && o ) = default;

    /**
     *
     */
    ~message_socket
        ( void );

    /**
     *
     */
    explicit
    message_socket
        ( message_socket const& o ) = delete;

    /**
     *
     */
    message_socket &
    operator=
        ( message_socket const& o ) = delete;

    /**
     *
     */
    template<typename ReceiveCallback>
    void
    async_receive
        ( ReceiveCallback const& callback );

    /**
     *
     */
    template<typename SendCallback>
    void
    async_send
        ( buffer const& message
        , endpoint_type const& to
        , SendCallback const& callback );

    /**
     *
     */
    endpoint_type
    local_endpoint
        ( void )
        const;

private:
    ///
    using underlying_socket_type = UnderlyingSocketType;

    ///
    using underlying_endpoint_type = typename underlying_socket_type::endpoint_type;

private:
    /**
     *
     */
    message_socket
        ( boost::asio::io_service & io_service
        , endpoint_type const& e );

    /**
     *
     */
    static underlying_socket_type
    create_underlying_socket
        ( boost::asio::io_service & io_service
        , endpoint_type const& e );

    /**
     *
     */
    static endpoint_type
    convert_endpoint
        ( underlying_endpoint_type const& e );

    /**
     *
     */
    static underlying_endpoint_type
    convert_endpoint
        ( endpoint_type const& e );

private:
    ///
    buffer reception_buffer_;
    ///
    underlying_endpoint_type current_message_sender_;
    ///
    underlying_socket_type socket_;
};

template< typename UnderlyingSocketType >
template< typename EndpointType >
inline message_socket< UnderlyingSocketType >
message_socket< UnderlyingSocketType >::ipv4
    ( boost::asio::io_service & io_service
    , EndpointType const& ipv4_endpoint )
{
    auto endpoints = resolve_endpoint( io_service, ipv4_endpoint );

    for ( auto const& i : endpoints )
    {
        if ( i.address_.is_v4() )
            return message_socket{ io_service, i };
    }

    throw std::system_error{ make_error_code( INVALID_IPV4_ADDRESS ) };
}

template< typename UnderlyingSocketType >
template< typename EndpointType >
inline message_socket< UnderlyingSocketType >
message_socket< UnderlyingSocketType >::ipv6
    ( boost::asio::io_service & io_service
    , EndpointType const& ipv6_endpoint )
{
    auto endpoints = resolve_endpoint( io_service, ipv6_endpoint );

    for ( auto const& i : endpoints )
    {
        if ( i.address_.is_v6() )
            return message_socket{ io_service, i };
    }

    throw std::system_error{ make_error_code( INVALID_IPV6_ADDRESS ) };
}

template< typename UnderlyingSocketType >
inline
message_socket< UnderlyingSocketType >::message_socket
    ( boost::asio::io_service & io_service
    , endpoint_type const& e )
    : reception_buffer_( INPUT_BUFFER_SIZE )
    , current_message_sender_()
    , socket_( create_underlying_socket( io_service, e ) )
{ }

template< typename UnderlyingSocketType >
inline
message_socket< UnderlyingSocketType >::~message_socket
    ( void )
{
    boost::system::error_code error_discared;
    socket_.close( error_discared );
}

template< typename UnderlyingSocketType >
template< typename ReceiveCallback >
inline void
message_socket< UnderlyingSocketType >::async_receive
    ( ReceiveCallback const& callback )
{
    auto on_completion = [ this, callback ]
        ( boost::system::error_code const& failure
        , std::size_t bytes_received )
    {
#ifdef _MSC_VER
        // On Windows, an UDP socket may return connection_reset
        // to inform application that a previous send by this socket
        // has generated an ICMP port unreachable.
        // https://msdn.microsoft.com/en-us/library/ms740120.aspx
        // Ignore it and schedule another read.
        if ( failure == boost::system::errc::connection_reset )
            return async_receive( callback );
#endif
        auto i = reception_buffer_.begin(), e = i;

        if ( ! failure )
            std::advance( e, bytes_received );

        callback( boost_to_std_error( failure )
                , convert_endpoint( current_message_sender_ )
                , i, e );
    };

    assert( reception_buffer_.size() == INPUT_BUFFER_SIZE );
    socket_.async_receive_from( boost::asio::buffer( reception_buffer_ )
                              , current_message_sender_
                              , std::move( on_completion ) );
}

template< typename UnderlyingSocketType >
template< typename SendCallback >
inline void
message_socket< UnderlyingSocketType >::async_send
    ( buffer const& message
    , endpoint_type const& to
    , SendCallback const& callback )
{
    if ( message.size() > INPUT_BUFFER_SIZE )
        callback( make_error_code( std::errc::value_too_large ) );
    else {
        // Copy the buffer as it has to live past the end of this call.
        auto message_copy = std::make_shared< buffer >( message );
        auto on_completion = [ this, callback, message_copy ]
            ( boost::system::error_code const& failure
            , std::size_t /* bytes_sent */ )
        {
            callback( boost_to_std_error( failure ) );
        };

        socket_.async_send_to( boost::asio::buffer( *message_copy )
                             , convert_endpoint( to )
                             , std::move( on_completion ) );
    }
}

template< typename UnderlyingSocketType >
inline typename message_socket< UnderlyingSocketType >::endpoint_type
message_socket< UnderlyingSocketType >::local_endpoint
    ( void )
    const
{
    return endpoint_type{ socket_.local_endpoint().address()
                        , socket_.local_endpoint().port() };
}

template< typename UnderlyingSocketType >
inline typename message_socket< UnderlyingSocketType >::underlying_socket_type
message_socket< UnderlyingSocketType >::create_underlying_socket
    ( boost::asio::io_service & io_service
    , endpoint_type const& endpoint )
{
    auto const e = convert_endpoint( endpoint );

    underlying_socket_type new_socket{ io_service, e.protocol() };

    if ( e.address().is_v6() )
        new_socket.set_option( boost::asio::ip::v6_only{ true } );

    new_socket.bind( e );

    return std::move( new_socket );
}

template< typename UnderlyingSocketType >
template< typename EndpointType >
inline typename message_socket< UnderlyingSocketType >::resolved_endpoints
message_socket< UnderlyingSocketType >::resolve_endpoint
    ( boost::asio::io_service & io_service
    , EndpointType const& e )
{
    using protocol_type = typename underlying_socket_type::protocol_type;

    typename protocol_type::resolver r{ io_service };
    // Resolve addresses even if not reachable.
    typename protocol_type::resolver::query::flags const f{};
    typename protocol_type::resolver::query q{ e.address(), e.service(), f };
    // One raw endpoint (e.g. localhost) can be resolved to
    // multiple endpoints (e.g. IPv4 / IPv6 address).
    resolved_endpoints endpoints;

    auto i = r.resolve( q );
    for ( decltype( i ) end; i != end; ++i )
        // Convert from underlying_endpoint_type to endpoint_type.
        endpoints.push_back( convert_endpoint( *i ) );

    return endpoints;
}

template< typename UnderlyingSocketType >
inline typename message_socket< UnderlyingSocketType >::endpoint_type
message_socket< UnderlyingSocketType >::convert_endpoint
    ( underlying_endpoint_type const& e )
{ return endpoint_type{ e.address(), e.port() }; }

template< typename UnderlyingSocketType >
inline typename message_socket< UnderlyingSocketType >::underlying_endpoint_type
message_socket< UnderlyingSocketType >::convert_endpoint
    ( endpoint_type const& e )
{ return underlying_endpoint_type{ e.address_, e.port_ }; }

} // namespace detail
} // namespace kademlia

#endif

