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

#include "simulator/application.hpp"

#include <memory>

#include <boost/asio/io_service.hpp>

#include "simulator/fake_socket.hpp"

#include "kademlia/buffer.hpp"
#include "kademlia/engine.hpp"

namespace kademlia {
namespace application {

namespace {

using test_engine = detail::engine< detail::buffer
                                  , detail::buffer
                                  , fake_socket >;

using engine_ptr = std::shared_ptr< test_engine >;

/**
 *
 */
template< typename BufferType >
detail::buffer
to_buffer
    ( BufferType const& b )
{ return detail::buffer( std::begin( b ), std::end( b ) ); }

/**
 *
 */
std::vector< engine_ptr >
create_engines
    ( boost::asio::io_service & io_service
    , configuration const& c )
{
    std::vector< engine_ptr > engines;
    for ( std::size_t i = 0; i != c.clients_count; ++i )
    {
        auto e = std::make_shared< test_engine >( io_service
                                                , endpoint( "0.0.0.0", 5555 )
                                                , endpoint( "::", 5555 ) );
        engines.push_back( e );
    }

    return std::move( engines );
}

/**
 *
 */
void
schedule_load
    ( engine_ptr const& e
    , std::size_t value )
{
    auto const b = to_buffer( std::to_string( value ) );

    auto check_load = [ b ] 
            ( std::error_code const& failure 
            , detail::buffer const& buffer )
    {
        if ( failure )
            throw std::system_error{ failure };

        if ( b != buffer )
            throw std::runtime_error{ "loaded value is incorrect" };
    };

    e->async_load( b, check_load );
}

/**
 *
 */
void
schedule_save
    ( engine_ptr const& e
    , std::size_t value )
{
    auto const b = to_buffer( std::to_string( value ) );

    auto check_save = [] 
            ( std::error_code const& failure )
    { 
        if ( failure ) 
            throw std::system_error{ failure }; 
    };

    e->async_save( b, b, check_save );
}

/**
 *
 */
void
schedule_tasks
    ( boost::asio::io_service & io_service
    , std::vector< engine_ptr > & engines
    , std::size_t total_messages_count )
{
    auto i = engines.begin(), e = engines.end();

    // For each message to send.
    for ( std::size_t sent_messages_count = 0
        ; sent_messages_count != total_messages_count
        ; ++ sent_messages_count )
    {
        // If the last peer has been reached, then
        // restart with the first one.
        if ( i == e ) 
            i = engines.begin();

        if ( sent_messages_count % 2 )
            schedule_load( *i, sent_messages_count - 1 );
        else
            schedule_save( *i, sent_messages_count );

        // Next task will be scheduled with the next peer.
        ++ i;
    }
}

} // anonymous namespace

void
run
    ( configuration const& c )
{
    boost::asio::io_service io_service;

    auto engines = create_engines( io_service, c );
    schedule_tasks( io_service, engines, c.total_messages_count );

    io_service.run();
}

} // namespace application
} // namespace kademlia

