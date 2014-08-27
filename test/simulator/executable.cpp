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

#include "simulator/executable.hpp"

#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <system_error>

#include <boost/program_options.hpp>

#include "simulator/configuration.hpp"
#include "simulator/application.hpp"
#include "kademlia/r.hpp"

namespace kademlia {
namespace executable {

namespace {

namespace po = boost::program_options;

detail::r< configuration >
parse_configuration
    ( int argc
    , char * argv[] )
{
    configuration c;

    po::options_description mandatory( "Mandatory arguments" );
    mandatory.add_options()
        ( "clients-count,c"
        , po::value< std::size_t >( &c.clients_count )
        , "Set the simulated clients count\n" )

        ( "messages-count,m"
        , po::value< std::size_t >( &c.total_messages_count )
        , "Set the total messages sent by simulated clients count\n" );

    po::options_description optional( "Misc arguments" );
    optional.add_options()
        ( "help,h", "Print accepted arguments\n" )

        ( "version,v", "Print version\n" );

    po::options_description all;
    all.add( mandatory ).add( optional );
    po::variables_map variables;
    po::store( po::parse_command_line( argc, argv, all ), variables );
    po::notify( variables );

    if ( variables.count( "version" ) )
    {
        std::cout << argv[ 0 ] << " version " PACKAGE_VERSION << std::endl;
        return make_error_code( std::errc::operation_canceled );
    }

    if ( variables.count( "help" ) )
    {
        std::cout << argv[ 0 ] << " usage:\n" << all << std::endl;
        return make_error_code( std::errc::operation_canceled );
    } 
    
    if ( ! variables.count( "clients-count" ) 
       || ! variables.count( "messages-count" ) )
        throw std::invalid_argument( "a required argument is missing" );

    return std::move( c );
}

} // anonymous namespace

int
run
    ( int argc
    , char * argv[] )
{
    auto configuration = parse_configuration( argc, argv );

    if ( ! configuration )
        return EXIT_SUCCESS;

    application::run( configuration.v() );

    return EXIT_SUCCESS;
}

} // namespace executable
} // namespace kademlia

