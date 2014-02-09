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

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include "common.hpp"

#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>

#include "message_socket.hpp"

namespace filesystem = boost::filesystem;
namespace unit_test = boost::unit_test;
namespace detail = kademlia::detail;

namespace {

filesystem::path captures_directory_;

} // namespace

std::string get_capture_path( std::string const & capture_name )
{
    return (captures_directory_ / capture_name).string();
}

std::uint16_t
get_temporary_listening_port
    ( std::uint16_t port )
{
    boost::system::error_code failure;

    do  
    {
        ++ port;
        detail::message_socket::endpoint_type const e
                { detail::message_socket::protocol_type::v4() , port }; 

        boost::asio::io_service io_service;
        // Try to open a socket at this address.
        detail::message_socket{ io_service, e.protocol() }.bind( e, failure );
    }
    while ( failure == boost::system::errc::address_in_use ); 

    return port;
}

int 
main
    ( int argc
    , char* argv[] )
{
    if ( argc == 1 )
    {
        std::cerr << "usage: " << argv[ 0 ] 
                  << " [test args] CAPTURE_DIR" << std::endl;
        return EXIT_FAILURE;
    }

    captures_directory_ = argv[ argc - 1 ];

    return boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}

