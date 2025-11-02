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

#ifndef KADEMLIA_SESSION_IMPL_HPP
#define KADEMLIA_SESSION_IMPL_HPP

#ifdef _MSC_VER
#   pragma once
#endif


#include "session_impl.hpp"

#include <utility>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>

#include "message_socket.hpp"
#include "engine.hpp"
#include "concurrent_guard.hpp"

namespace kademlia {
namespace detail {

/**
 *
 */
class session_impl
{
public:
    ///
    using data_type = std::vector< std::uint8_t >;
    ///
    using key_type = std::vector< std::uint8_t >;
    ///
    using socket_type = boost::asio::ip::udp::socket;
    ///
    using engine_type = detail::engine< socket_type >;

public:
    /**
     *
     */
    session_impl
        ( endpoint const& listen_on_ipv4
        , endpoint const& listen_on_ipv6 )
            : io_service_{}
            , engine_{ io_service_
                     , listen_on_ipv4
                     , listen_on_ipv6 }
            , is_abort_requested_{}
            , concurrent_guard_{}
    { }

    /**
     *
     */
    session_impl
        ( endpoint const& initial_peer
        , endpoint const& listen_on_ipv4
        , endpoint const& listen_on_ipv6 )
            : io_service_{}
            , engine_{ io_service_
                     , initial_peer
                     , listen_on_ipv4
                     , listen_on_ipv6 }
            , is_abort_requested_{}
            , concurrent_guard_{}
    { }

    /**
     *
     */
    template< typename HandlerType >
    void
    async_save
        ( key_type const& key
        , data_type const& data
        , HandlerType && handler )
    {
        engine_.async_save( key
                          , data
                          , std::forward< HandlerType >( handler ) );
    }

    /**
     *
     */
    template< typename HandlerType >
    void
    async_load
        ( key_type const& key
        , HandlerType && handler )
    {
        engine_.async_load( key
                          , std::forward< HandlerType >( handler ) );
    }

    /**
     *
     */
    std::error_code
    run
        ( void )
    {
        // Protect against concurrent invocation of this method.
        detail::concurrent_guard::sentry s{ concurrent_guard_ };
        if ( ! s )
            return make_error_code( ALREADY_RUNNING );

        is_abort_requested_ = false;

        while ( ! is_abort_requested_ )
        {
            io_service_.run_one();
            io_service_.poll();
        }

        return make_error_code( RUN_ABORTED );
    }

    /**
     *
     */
    void
    abort
        ( void )
    {
        auto service_stopper = [ this ] ( void )
        { is_abort_requested_ = true; };

        io_service_.post( service_stopper );
    }

private:
    ///
    boost::asio::io_service io_service_;
    ///
    engine_type engine_;
    ///
    bool is_abort_requested_;
    ///
    detail::concurrent_guard concurrent_guard_;
};

} // namespace detail
} // namespace kademlia

#endif

