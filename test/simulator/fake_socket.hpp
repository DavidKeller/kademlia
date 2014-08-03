// Copyright (c) 2014, David Keller
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

#ifndef KADEMLIA_SESSION_HPP
#define KADEMLIA_SESSION_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>

/**
 *
 */
class fake_socket
{
public:
    ///
    using endpoint_type = boost::asio::ip::udp::endpoint;

    ///
    using protocol_type = boost::asio::ip::udp;

public:
    /**
     *
     */
    fake_socket
        ( boost::asio::io_service & io_service
        , boost::asio::ip::udp const & protocol )
        : io_service_( io_service )
    { }

    /**
     *
     */
    template< typename Option >
    void 
    set_option
        ( Option const& )
    { }

    /**
     *
     */
    boost::system::error_code
    bind
        ( endpoint_type const& )
    { return boost::system::error_code{}; }

    /**
     *
     */
    boost::system::error_code
    close
        ( boost::system::error_code & failure )
    { return failure; }

    /**
     *
     */
    template< typename Callback >
    boost::system::error_code
    async_receive_from
        ( boost::asio::mutable_buffer const& 
        , endpoint_type & 
        , Callback )
    { return boost::system::error_code{}; }

    /**
     *
     */
    template< typename Callback >
    boost::system::error_code
    async_send_to
        ( boost::asio::const_buffer const& 
        , endpoint_type const& 
        , Callback )
    { return boost::system::error_code{}; }

private:
    ///
    boost::asio::io_service & io_service_;
};

#endif

