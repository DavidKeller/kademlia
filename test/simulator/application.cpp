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

#include "kademlia/log.hpp"
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

    endpoint const ipv4_listen( "0.0.0.0", fake_socket::FIXED_PORT );
    endpoint const ipv6_listen( "::", fake_socket::FIXED_PORT );

    // Create bootstrap node.
    {
        auto e = std::make_shared< test_engine >( io_service
                                                , ipv4_listen
                                                , ipv6_listen );
        engines.push_back( e );
    }

    // Get the address of the bootstrap node.
    auto const first_peer_ipv4
            = fake_socket::get_last_allocated_ipv4().to_string();
    endpoint const first_peer( first_peer_ipv4
                             , fake_socket::FIXED_PORT );

    // Create nodes.
    for ( std::size_t i = 0ULL; i != c.clients_count; ++i )
    {
        auto e = std::make_shared< test_engine >( io_service
                                                , first_peer
                                                , ipv4_listen
                                                , ipv6_listen );
        engines.push_back( e );

        io_service.poll();
    }

    return std::move( engines );
}

/**
 *
 */
void
schedule_load
    ( engine_ptr const& e
    , std::size_t value
    , std::size_t & received_messages_count )
{
    auto const b = to_buffer( std::to_string( value ) );

    auto check_load = [ b, &received_messages_count ]
            ( std::error_code const& failure
            , detail::buffer const& buffer )
    {
        if ( failure )
            throw std::system_error{ failure };

        if ( b != buffer )
            throw std::runtime_error{ "loaded value is incorrect" };

        ++ received_messages_count;

        LOG_DEBUG( simulator, nullptr ) << "received message id '"
                << received_messages_count
                << "'." << std::endl;
    };

    e->async_load( b, check_load );
}

/**
 *
 */
template< typename Engines >
void
schedule_loads
    ( Engines const& engines
    , boost::asio::io_service & io_service
    , std::size_t total_messages_count )
{
    LOG_DEBUG( simulator, nullptr ) << "loading '"
            << total_messages_count
            << "' messages." << std::endl;

    std::size_t received_messages_count = 0ULL;
    for ( auto i = 0ULL; i != total_messages_count; ++i )
        schedule_load( engines[ i % engines.size() ]
                     , i
                     , received_messages_count );

    while ( total_messages_count != received_messages_count )
        io_service.run_one();
}

/**
 *
 */
void
schedule_save
    ( engine_ptr const& e
    , std::size_t value
    , std::size_t & sent_messages_count )
{
    auto const b = to_buffer( std::to_string( value ) );

    auto check_save = [ &sent_messages_count ]
            ( std::error_code const& failure )
    {
        if ( failure )
            throw std::system_error{ failure };

        ++ sent_messages_count;

        LOG_DEBUG( simulator, nullptr ) << "sent message id '"
                << sent_messages_count
                << "'." << std::endl;
    };

    e->async_save( b, b, check_save );
}

/**
 *
 */
template< typename Engines >
void
schedule_saves
    ( Engines const& engines
    , boost::asio::io_service & io_service
    , std::size_t total_messages_count )
{
    LOG_DEBUG( simulator, nullptr ) << "saving '"
            << total_messages_count
            << "' messages." << std::endl;

    std::size_t sent_messages_count = 0ULL;
    for ( auto i = 0ULL; i != total_messages_count; ++i )
        schedule_save( engines[ i % engines.size() ]
                     , i
                     , sent_messages_count );

    while ( total_messages_count != sent_messages_count )
        io_service.run_one();
}

} // anonymous namespace

void
run
    ( configuration const& c )
{
#if KADEMLIA_ENABLE_DEBUG
    for ( auto const& module : c.log_modules )
        detail::enable_log_for( module );
#endif

    boost::asio::io_service io_service;

    std::cout << "Creating peers" << std::endl;
    auto engines = create_engines( io_service, c );

    std::cout << "Performing saves" << std::endl;
    schedule_saves( engines, io_service, c.total_messages_count );

    std::cout << "Perfoming loads" << std::endl;
    schedule_loads( engines, io_service, c.total_messages_count );

    std::cout <<  "Messages count: "
              << fake_socket::get_writes_count()
              << std::endl;
}

} // namespace application
} // namespace kademlia

