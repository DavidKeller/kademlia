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

#include "network.hpp"

#include <boost/asio/ip/udp.hpp>

namespace kademlia {
namespace test {

void
check_listening
    ( std::string const& ip
    , std::uint16_t port )
{
    using boost::asio::ip::udp;
    auto udp_failure = create_socket< udp::socket >( ip, port );
    BOOST_REQUIRE_EQUAL( boost::system::errc::address_in_use, udp_failure );
}

std::uint16_t
get_temporary_listening_port
    ( std::uint16_t port )
{
    boost::system::error_code failure;

    do
    {
        ++ port;
        boost::asio::ip::udp::endpoint const e
                { boost::asio::ip::udp::v4() , port };

        boost::asio::io_service io_service;
        // Try to open a socket at this address.
        boost::asio::ip::udp::socket socket{ io_service, e.protocol() };
        socket.bind( e, failure );
    }
    while ( failure == boost::system::errc::address_in_use );

    return port;
}

} // namespace test
} // namespace kademlia

