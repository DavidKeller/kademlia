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

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4996)
#endif

#include <kademlia/session.hpp>

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <functional>
#include <boost/asio/io_service.hpp>

#include <kademlia/error.hpp>

#include "message_socket.hpp"
#include "routing_table.hpp"
#include "subnet.hpp"

namespace kademlia {

/**
 *
 */
class session::impl final
{
public:
    ///
    using subnets = std::vector<detail::subnet>;

public:
    /**
     *
     */
    explicit 
    impl
        ( std::vector<endpoint > const& endpoints
        , endpoint const& initial_peer )
            : io_service_{}
            , subnets_{ create_subnets( detail::create_sockets( io_service_, endpoints ) ) }
            , routing_table_{}
            , failure_{}
    {
        init( initial_peer );
    }

    /**
     *
     */
    void
    async_save
        ( key_type const& key 
        , data_type const& data
        , save_handler_type handler )
    { throw std::system_error{ make_error_code( UNIMPLEMENTED ) }; }

    /**
     *
     */
    void
    async_load
        ( key_type const& key
        , load_handler_type handler )
    { throw std::system_error{ make_error_code( UNIMPLEMENTED ) }; }

    /**
     *
     */
    std::error_code
    run
        ( void ) 
    { 
        failure_.clear();
        io_service_.reset();

        schedule_initial_receive_on_each_subnet();

        while ( ! failure_ && io_service_.run_one() );

        return failure_;
    }

    /**
     *
     */
    void
    abort
        ( void )
    { 
        io_service_.stop();
        failure_ = make_error_code( RUN_ABORTED );
    }

private:
    /**
     *
     */
    static subnets
    create_subnets
        ( detail::message_sockets sockets ) 
    {
        subnets new_subnets;

        for ( auto & current_socket : sockets )
            new_subnets.emplace_back( std::move( current_socket ) );

        return new_subnets;
    }

    /**
     *
     */
    void
    init
        ( endpoint const& initial_peer )
    { 
        for ( auto & current_endpoint : detail::resolve_endpoint( io_service_, initial_peer ) )
            // XXX: Sens a FIND_NODE with our own id to this endpoint.
            (void)current_endpoint;
    }

    /**
     *
     */
    void
    schedule_initial_receive_on_each_subnet
        ( void )
    {
        for ( auto & current_subnet : subnets_ )
            schedule_receive_on_subnet( current_subnet );
    }

    /**
     *
     */
    void
    schedule_receive_on_subnet
        ( detail::subnet & current_subnet )
    {
        auto on_new_message = [ this, &current_subnet ]
            ( std::error_code const& failure
            , detail::message_socket::endpoint_type const& sender
            , detail::buffer const& message )
        {
            if ( failure )
                failure_ = failure;
            else
            {
                process_new_message( current_subnet, sender, message );
                schedule_receive_on_subnet( current_subnet );
            }
        };

        current_subnet.async_receive( on_new_message );
    }

    /**
     *
     */
    void
    process_new_message
        ( detail::subnet & source_subnet
        , detail::message_socket::endpoint_type const& sender
        , detail::buffer const& message )
    {

    }

private:
    boost::asio::io_service io_service_;
    subnets subnets_;
    detail::routing_table routing_table_;
    std::error_code failure_;
};

session::session
    ( std::vector< endpoint > const& endpoints
    , endpoint const& initial_peer )
    : impl_{ new impl{ endpoints, initial_peer } }
{ }

session::~session
    ( void )
{ }

void
session::async_save
    ( key_type const& key 
    , data_type const& data
    , save_handler_type handler )
{ impl_->async_save( key, data, handler ); }

void
session::async_load
    ( key_type const& key
    , load_handler_type handler )
{ impl_->async_load( key, handler ); }

std::error_code
session::run
        ( void )
{ return impl_->run(); }

void
session::abort
        ( void )
{ impl_->abort(); }

} // namespace kademlia

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif
