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

#ifndef KADEMLIA_ENGINE_HPP
#define KADEMLIA_ENGINE_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <algorithm>
#include <stdexcept>
#include <queue>
#include <chrono>
#include <random>
#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include <boost/asio/io_service.hpp>

#include <kademlia/endpoint.hpp>
#include "kademlia/error_impl.hpp"

#include "kademlia/log.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/message_serializer.hpp"
#include "kademlia/response_router.hpp"
#include "kademlia/network.hpp"
#include "kademlia/message.hpp"
#include "kademlia/routing_table.hpp"
#include "kademlia/value_store.hpp"
#include "kademlia/find_value_task.hpp"
#include "kademlia/store_value_task.hpp"
#include "kademlia/discover_neighbors_task.hpp"
#include "kademlia/notify_peer_task.hpp"
#include "kademlia/tracker.hpp"

namespace kademlia {
namespace detail {

/**
 *
 */
template< typename UnderlyingSocketType >
class engine final
{
public:
    ///
    using key_type = std::vector< std::uint8_t >;

    ///
    using data_type = std::vector< std::uint8_t >;

    ///
    using endpoint_type = ip_endpoint;

    ///
    using routing_table_type = routing_table< endpoint_type >;

    ///
    using value_store_type = value_store< id, data_type >;

public:
    /**
     *
     */
    engine
        ( boost::asio::io_service & io_service
        , endpoint const& ipv4
        , endpoint const& ipv6
        , id const& new_id = id{} )
            : random_engine_( std::random_device{}() )
            , my_id_( new_id == id{} ? id{ random_engine_ } : new_id )
            , network_( io_service
                      , message_socket_type::ipv4( io_service, ipv4 )
                      , message_socket_type::ipv6( io_service, ipv6 )
                      , std::bind( &engine::handle_new_message
                                 , this
                                 , std::placeholders::_1
                                 , std::placeholders::_2
                                 , std::placeholders::_3 ) )
            , tracker_( io_service
                      , my_id_
                      , network_
                      , random_engine_ )
            , routing_table_( my_id_ )
            , value_store_()
            , is_connected_()
            , pending_tasks_()
    { }

    /**
     *
     */
    engine
        ( boost::asio::io_service & io_service
        , endpoint const& initial_peer
        , endpoint const& ipv4
        , endpoint const& ipv6
        , id const& new_id = id{} )
            : engine( io_service, ipv4, ipv6, new_id )
    {
        LOG_DEBUG( engine, this ) << "bootstrapping using peer '"
                << initial_peer << "'." << std::endl;

        discover_neighbors( initial_peer );
    }

    /**
     *
     */
    engine
        ( engine const& )
        = delete;

    /**
     *
     */
    engine &
    operator=
        ( engine const& )
        = delete;

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
        // If the routing table is empty, save the
        // current request for processing when
        // the routing table will be filled.
        if ( ! is_connected_ )
        {
            LOG_DEBUG( engine, this ) << "delaying async save of key '"
                    << to_string( key ) << "'." << std::endl;

            auto t = [ this, key, data, handler ] ( void ) mutable
            { async_save( key, data, std::move( handler ) ); };

            pending_tasks_.push( std::move( t ) );
        }
        else
        {
            LOG_DEBUG( engine, this ) << "executing async save of key '"
                    << to_string( key ) << "'." << std::endl;

            start_store_value_task( id( key )
                                  , data
                                  , tracker_
                                  , routing_table_
                                  , std::forward< HandlerType >( handler ) );
        }
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
        // If the routing table is empty, save the
        // current request for processing when
        // the routing table will be filled.
        if ( ! is_connected_ )
        {
            LOG_DEBUG( engine, this ) << "delaying async load of key '"
                    << to_string( key ) << "'." << std::endl;

            auto t = [ this, key, handler ] ( void ) mutable
            { async_load( key, std::move( handler ) ); };

            pending_tasks_.push( std::move( t ) );
        }
        else
        {
            LOG_DEBUG( engine, this ) << "executing async load of key '"
                    << to_string( key ) << "'." << std::endl;

            start_find_value_task< data_type >( id( key )
                                              , tracker_
                                              , routing_table_
                                              , std::forward< HandlerType >( handler ) );
        }
    }

private:
    ///
    using pending_task_type = std::function< void ( void ) >;

    ///
    using message_socket_type = message_socket< UnderlyingSocketType >;

    ///
    using network_type = network< message_socket_type >;

    ///
    using random_engine_type = std::default_random_engine;

    ///
    using tracker_type = tracker< random_engine_type, network_type >;

private:
    /**
     *
     */
    void
    process_new_message
        ( ip_endpoint const& sender
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    {
        switch ( h.type_ )
        {
            case header::PING_REQUEST:
                handle_ping_request( sender, h );
                break;
            case header::STORE_REQUEST:
                handle_store_request( sender, h, i, e );
                break;
            case header::FIND_PEER_REQUEST:
                handle_find_peer_request( sender, h, i, e );
                break;
            case header::FIND_VALUE_REQUEST:
                handle_find_value_request( sender, h, i, e );
                break;
            default:
                tracker_.handle_new_response( sender, h, i, e );
                break;
        }
    }

    /**
     *
     */
    void
    handle_ping_request
        ( ip_endpoint const& sender
        , header const& h )
    {
        LOG_DEBUG( engine, this ) << "handling ping request." << std::endl;

        tracker_.send_response( h.random_token_
                              , header::PING_RESPONSE
                              , sender );
    }

    /**
     *
     */
    void
    handle_store_request
        ( ip_endpoint const& sender
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    {
        LOG_DEBUG( engine, this ) << "handling store request."
                << std::endl;

        store_value_request_body request;
        if ( auto failure = deserialize( i, e, request ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize store value request ("
                    << failure.message() << ")." << std::endl;

            return;
        }

        value_store_[ request.data_key_hash_ ]
                = std::move( request.data_value_ );
    }

    /**
     *
     */
    void
    handle_find_peer_request
        ( ip_endpoint const& sender
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    {
        LOG_DEBUG( engine, this ) << "handling find peer request."
                << std::endl;

        // Ensure the request is valid.
        find_peer_request_body request;
        if ( auto failure = deserialize( i, e, request ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize find peer request ("
                    << failure.message() << ")" << std::endl;

            return;
        }

        send_find_peer_response( sender
                               , h.random_token_
                               , request.peer_to_find_id_ );
    }

    /**
     *
     */
    void
    send_find_peer_response
        ( ip_endpoint const& sender
        , id const& random_token
        , id const& peer_to_find_id )
    {
        // Find X closest peers and save
        // their location into the response..
        find_peer_response_body response;

        auto remaining_peer = ROUTING_TABLE_BUCKET_SIZE;
        for ( auto i = routing_table_.find( peer_to_find_id )
                 , e = routing_table_.end()
            ; i != e && remaining_peer > 0
            ; ++i, -- remaining_peer )
            response.peers_.push_back( { i->first, i->second } );

        // Now send the response.
        tracker_.send_response( random_token, response, sender );
    }

    /**
     *
     */
    void
    handle_find_value_request
        ( ip_endpoint const& sender
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    {
        LOG_DEBUG( engine, this ) << "handling find value request."
                << std::endl;

        find_value_request_body request;
        if ( auto failure = deserialize( i, e, request ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize find value request ("
                    << failure.message() << ")" << std::endl;

            return;
        }

        auto found = value_store_.find( request.value_to_find_ );
        if ( found == value_store_.end() )
            send_find_peer_response( sender
                                   , h.random_token_
                                   , request.value_to_find_ );
        else
        {
            find_value_response_body const response{ found->second };
            tracker_.send_response( h.random_token_
                                  , response
                                  , sender );
        }
    }

    /**
     *
     */
    void
    discover_neighbors
        ( endpoint const& initial_peer )
    {
        // Initial peer should know our neighbors, hence ask
        // him which peers are close to our own id.
        auto endoints_to_query = network_.resolve_endpoint( initial_peer );

        auto on_discovery = [ this ]
            ( std::error_code const& failure )
        {
            if ( failure )
                throw std::system_error{ failure };

            notify_neighbors();
        };

        start_discover_neighbors_task( my_id_, tracker_, routing_table_
                                     , std::move( endoints_to_query )
                                     , on_discovery );
    }

    /**
     *
     */
    id
    get_closest_neighbor_id
        ( void )
    {
        // Find our closest neighbor.
        auto closest_neighbor = routing_table_.find( my_id_ );
        if ( closest_neighbor->first == my_id_ )
            ++ closest_neighbor;

        assert( closest_neighbor != routing_table_.end()
              && "at least one peer is known" );

        return closest_neighbor->first;
    }

    /**
     *  Refresh each bucket.
     */
    void
    notify_neighbors
        ( void )
    {
        auto closest_neighbor_id = get_closest_neighbor_id();
        auto i = id::BIT_SIZE - 1;

        // Skip empty buckets.
        while ( i && closest_neighbor_id[ i ] == my_id_[ i ] )
            -- i;

        // Send refresh from closest neighbor bucket to farest bucket.
        auto refresh_id = my_id_;
        while ( i )
        {
            refresh_id[ i ] = ! refresh_id[ i ];
            start_notify_peer_task( refresh_id
                                  , tracker_, routing_table_ );
            -- i;
        }
    }

    /**
     *
     */
    void
    handle_new_message
        ( ip_endpoint const& sender
        , buffer::const_iterator i
        , buffer::const_iterator e  )
    {
        LOG_DEBUG( engine, this ) << "received new message from '"
                << sender << "'." << std::endl;

        detail::header h;
        // Try to deserialize header.
        if ( auto failure = deserialize( i, e, h ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize message header ("
                    << failure.message() << ")" << std::endl;
            return;
        }

        routing_table_.push( h.source_id_, sender );

        process_new_message( sender, h, i, e );

        // A message has been received, hence the connection
        // is up. Check if it was down before.
        if ( ! is_connected_ )
        {
            is_connected_ = true;
            execute_pending_tasks();
        }
    }

    /**
     *
     */
    void
    execute_pending_tasks
        ( void )
    {
        LOG_DEBUG( engine, this ) << "execute '" << pending_tasks_.size()
                << "' pending task(s)." << std::endl;

        // Some store/find requests may be pending
        // while the initial peer was contacted.
        while ( ! pending_tasks_.empty() )
        {
            pending_tasks_.front()();
            pending_tasks_.pop();
        }
    }

private:
    ///
    random_engine_type random_engine_;
    ///
    id my_id_;
    ///
    network_type network_;
    ///
    tracker_type tracker_;
    ///
    routing_table_type routing_table_;
    ///
    value_store_type value_store_;
    ///
    bool is_connected_;
    ///
    std::queue< pending_task_type > pending_tasks_;
};

} // namespace detail
} // namespace kademlia

#endif
