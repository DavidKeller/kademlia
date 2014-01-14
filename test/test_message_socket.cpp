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

#include "message_socket.hpp"

#include "helpers/common.hpp"

namespace k = kademlia;

/**
 *
 */
BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( one_socket_per_address_is_created )
{
    auto const port = get_temporary_listening_port();
    
    std::vector<k::endpoint> endpoints; 
    endpoints.emplace_back( "127.0.0.1", port );
    endpoints.emplace_back( "::1", port );
    
    boost::asio::io_service io_service;
    auto sockets = k::detail::create_sockets( io_service, endpoints );
    BOOST_REQUIRE_EQUAL( 2, sockets.size() );
}

BOOST_AUTO_TEST_SUITE_END()
