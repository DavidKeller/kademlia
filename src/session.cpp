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

#ifdef _MSC_VER
#   pragma warning( push )
#   pragma warning( disable : 4996 )
#endif

#include <kademlia/session.hpp>
#include <kademlia/error.hpp>

#include <list>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <functional>
#include <chrono>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <kademlia/error.hpp>

#include "message_socket.hpp"
//#include "task.hpp"
#include "routing_table.hpp"
#include "subnet.hpp"

namespace kademlia {

namespace {

CXX11_CONSTEXPR std::chrono::milliseconds TICK_TIMER_RESOLUTION{ 1 };
CXX11_CONSTEXPR std::chrono::milliseconds INITIAL_CONTACT_RECEIVE_TIMEOUT{ 1000 };

} // anonymous namespace

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
        ( std::vector<endpoint> const& endpoints
        , endpoint const& initial_peer )
            : my_id_{ detail::generate_id() }
            , io_service_{}
            , initial_peer_{ initial_peer }
            , tick_timer_{ io_service_ }
            , subnets_{ create_subnets( detail::create_sockets( io_service_, endpoints ) ) }
            , routing_table_{ my_id_ }
            , tasks_{}
            , main_failure_{}
    { }
    
    /**
     *
     */
    void
    start_tick_timer
        ( void )
    {
        tick_timer_.expires_from_now( TICK_TIMER_RESOLUTION );
        auto on_fire = [ this ]
            ( boost::system::error_code const& failure )
        { 
            if ( ! failure )
                start_tick_timer();
        };

        tick_timer_.async_wait( on_fire );
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
        main_failure_.clear();
        init();
        
        while ( ! main_failure_ && io_service_.run_one() ) 
            execute_tasks();

        return main_failure_;
    }

    /**
     *
     */
    void
    abort
        ( void )
    { 
        io_service_.stop();

        main_failure_ = make_error_code( RUN_ABORTED );
    }

private:
    ///
    using task = std::function<std::error_code ( void )>;

private:
    /**
     *
     */
    void
    init
        ( void )
    {
        io_service_.reset();

        start_tick_timer();
        start_receive_on_each_subnet();
        contact_initial_peer();
    }

    /**
     *
     */
    void
    execute_tasks
        ( void )
    {
        auto is_task_finished = []
            ( task & current_task )
        { 
            return current_task() != std::errc::operation_in_progress; 
        };

        tasks_.remove_if( is_task_finished );
    }

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
    contact_initial_peer
        ( void )
    {
        auto last_request_sending_time = std::chrono::steady_clock::now();
        auto current_subnet = subnets_.begin();
        auto endpoints_to_try = detail::resolve_endpoint( io_service_, initial_peer_ );
        auto new_task = [ this, last_request_sending_time, current_subnet, endpoints_to_try ] 
            ( void ) mutable -> std::error_code
        { 
            // If the routing_table has been filled, we can leave this method.
            if ( routing_table_.peer_count() != 0 )
                return std::error_code{};

            // If timeout is not yet elasped, return without trying another endpoint/subnet.
            auto const timeout = std::chrono::steady_clock::now() - last_request_sending_time; 
            if ( timeout <= INITIAL_CONTACT_RECEIVE_TIMEOUT )
                return make_error_code( std::errc::operation_in_progress );

            // Current endpoint didn't respond after timeout,
            // hence try with another one.
            for (
                ; ! endpoints_to_try.empty()
                ; endpoints_to_try.pop_back() )
                // Try first compatible subnet.
                for ( 
                    ; current_subnet != subnets_.end()
                    ; ++ current_subnet )
                {
                    // Try to send the request, returning a non zero std::error_code on error.
                    if ( send_initial_request( endpoints_to_try.back(), *current_subnet ) )
                        continue;

                    last_request_sending_time = std::chrono::steady_clock::now();

                    // And exit.
                    return make_error_code( std::errc::operation_in_progress );
                }
                
                current_subnet = subnets_.begin();

            // No responsive endpoint has been found.
            main_failure_ = make_error_code( INITIAL_PEER_FAILED_TO_RESPOND );
            return make_error_code( std::errc::timed_out ); 
        };

        tasks_.emplace_back( std::move( new_task ) );
    }

    /**
     *
     */
    void
    start_receive_on_each_subnet
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
                main_failure_ = failure;
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
    std::error_code 
    send_initial_request
        ( detail::message_socket::endpoint_type const& endpoint_to_try
        , detail::subnet & current_subnet )
    {
        // Ensure we can access to the current peer address
        // on the current subnet (i.e. we don't try
        // to reach an IPv4 peer from an IPv6 socket).
        if ( endpoint_to_try.protocol() != current_subnet.local_endpoint().protocol() )
            make_error_code( std::errc::address_family_not_supported );

        auto message = generate_initial_request();

        auto on_message_sent = [ message ]
            ( std::error_code const& /* failure */ )
        { };
        current_subnet.async_send( *message, endpoint_to_try, on_message_sent );

        return std::error_code{};
    } 

    /**
     *
     */
    std::shared_ptr<detail::buffer>
    generate_initial_request
        ( void )
    {
        return std::make_shared<detail::buffer>();
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
    detail::id my_id_;
    boost::asio::io_service io_service_;
    endpoint initial_peer_;
    boost::asio::steady_timer tick_timer_;
    subnets subnets_;
    detail::routing_table routing_table_;
    std::list<task> tasks_;
    std::error_code main_failure_;
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

#ifdef _MSC_VER
#   pragma warning( pop )
#endif
