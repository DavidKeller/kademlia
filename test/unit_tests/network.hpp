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

#ifndef KADEMLIA_TEST_HELPERS_NETWORK_HPP
#define KADEMLIA_TEST_HELPERS_NETWORK_HPP

#include <cstdint>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/system/system_error.hpp>

#include "common.hpp"

namespace kademlia {
namespace test {

template< typename Socket >
boost::system::error_code
create_socket
    ( std::string const& ip
    , std::uint16_t port )
{
    auto const a = boost::asio::ip::address::from_string( ip );
    boost::asio::io_service io_service;

    // Try to create a socket.
    typename Socket::endpoint_type endpoint( a, port );
    Socket socket( io_service, endpoint.protocol() );

    if ( endpoint.address().is_v6() )
        socket.set_option( boost::asio::ip::v6_only{ true } );

    boost::system::error_code failure;
    socket.bind( endpoint, failure );

    return failure;
}

void
check_listening
    ( std::string const& ip
    , std::uint16_t port );

std::uint16_t
get_temporary_listening_port
    ( std::uint16_t port = 1234 );

} // namespace test
} // namespace kademlia

#endif

