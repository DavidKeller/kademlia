// Copyright (c) 2013, David Keller
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

#ifndef KADEMLIA_SUBNET_HPP
#define KADEMLIA_SUBNET_HPP

#if defined(_MSC_VER)
#   pragma once
#endif

#include <kademlia/detail/cxx11_macros.hpp>

#include "buffer.hpp"
#include "message_socket.hpp"
#include "boost_to_std_error.hpp"

namespace kademlia {
namespace detail {

/**
 *
 */
class subnet final
{
public:
    /// Consider we won't receive IPv6 jumbo datagram.
    static CXX11_CONSTEXPR std::size_t INPUT_BUFFER_SIZE = UINT16_MAX;

public:
    /**
     *
     */
    explicit
    subnet
        ( message_socket subnet_socket );

    /**
     *
     */
    explicit
    subnet
#ifdef _MSC_VER
        ( subnet && o )
        : reception_buffer_{ std::move( o.reception_buffer_ ) }
        , current_message_sender_{ std::move( o.current_message_sender_ ) }
        , socket_{ std::move( o.socket_ ) }
    { }
#else
        ( subnet && o ) = default;
#endif

    /**
     *
     */
    explicit
    subnet
        ( subnet const& o ) = delete;

    /**
     *
     */
    subnet &
    operator=
        ( subnet const& o ) = delete;

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
        , message_socket::endpoint_type const& to
        , SendCallback const& callback );

private:
    ///
    buffer reception_buffer_;
    ///
    message_socket::endpoint_type current_message_sender_;
    ///
    message_socket socket_;
};

inline subnet::subnet
    ( message_socket subnet_socket )
    : reception_buffer_{}
    , current_message_sender_{}
    , socket_{ std::forward<message_socket>( subnet_socket ) }
{ }

template<typename ReceiveCallback>
inline void
subnet::async_receive
    ( ReceiveCallback const& callback )
{
    auto on_completion = [ this, callback ]
        ( boost::system::error_code const& failure
        , std::size_t bytes_received )
    {
        if ( failure ) 
            this->reception_buffer_.resize( 0 );
        else
            this->reception_buffer_.resize( bytes_received );

        callback( boost_to_std_error( failure )
                , this->current_message_sender_
                , this->reception_buffer_ );
    };

    this->reception_buffer_.resize( INPUT_BUFFER_SIZE );

    this->socket_.async_receive_from( boost::asio::buffer( this->reception_buffer_ )
                                    , this->current_message_sender_
                                    , on_completion );
}

template<typename SendCallback>
inline void
subnet::async_send
    ( buffer const& message
    , message_socket::endpoint_type const& to
    , SendCallback const& callback )
{
    if ( message.size() > INPUT_BUFFER_SIZE )
        callback( make_error_code( std::errc::value_too_large ) );
    else {
        auto on_completion = [ this, callback ]
            ( boost::system::error_code const& failure
            , std::size_t /* bytes_sent */ )
        {
            callback( boost_to_std_error( failure ) );
        };

        this->socket_.async_send_to( boost::asio::buffer( message )
                                   , to
                                   , on_completion );
    }
}

} // namespace detail
} // namespace kademlia

#endif

