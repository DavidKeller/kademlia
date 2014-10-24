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
#include <boost/asio/io_service.hpp>

#include <kademlia/endpoint.hpp>
#include <kademlia/error.hpp>

#include "kademlia/log.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/message_serializer.hpp"
#include "kademlia/response_router.hpp"
#include "kademlia/message_socket.hpp"
#include "kademlia/message.hpp"
#include "kademlia/routing_table.hpp"
#include "kademlia/value_store.hpp"
#include "kademlia/find_value_context.hpp"
#include "kademlia/store_value_context.hpp"
#include "kademlia/notify_peer_context.hpp"

namespace kademlia {
namespace detail {

namespace {

// k
CXX11_CONSTEXPR std::size_t ROUTING_TABLE_BUCKET_SIZE{ 20 };
// a
CXX11_CONSTEXPR std::size_t CONCURRENT_FIND_PEER_REQUESTS_COUNT{ 3 };
// c
CXX11_CONSTEXPR std::size_t REDUNDANT_SAVE_COUNT{ 3 };

//
CXX11_CONSTEXPR std::chrono::milliseconds INITIAL_CONTACT_RECEIVE_TIMEOUT{ 1000 };
//
CXX11_CONSTEXPR std::chrono::milliseconds PEER_LOOKUP_TIMEOUT{ 20 };

} // anonymous namespace

/**
 *
 */
template< typename KeyType, typename DataType, typename UnderlyingSocketType >
class engine final
{
public:
    ///
    using key_type = KeyType;

    ///
    using data_type = DataType;

    ///
    using message_socket_type = message_socket< UnderlyingSocketType >;

public:
    /**
     *
     */
    engine
        ( boost::asio::io_service & io_service
        , endpoint const& ipv4
        , endpoint const& ipv6 )
            : random_engine_{ std::random_device{}() }
            , my_id_( random_engine_ )
            , io_service_( io_service )
            , response_router_( io_service )
            , message_serializer_( my_id_ )
            , socket_ipv4_{ message_socket_type::ipv4( io_service_, ipv4 ) }
            , socket_ipv6_{ message_socket_type::ipv6( io_service_, ipv6 ) }
            , routing_table_{ my_id_ }
            , value_store_{}
            , is_connected_{}
            , pending_tasks_{}
    {
        start_message_reception();
        LOG_DEBUG( engine, this ) << "created at '"
                << socket_ipv4_.local_endpoint() << "' and '"
                << socket_ipv6_.local_endpoint() << "'." << std::endl;
    }

    /**
     *
     */
    engine
        ( boost::asio::io_service & io_service
        , endpoint const& initial_peer
        , endpoint const& ipv4
        , endpoint const& ipv6 )
            : random_engine_{ std::random_device{}() }
            , my_id_( random_engine_ )
            , io_service_( io_service )
            , response_router_( io_service )
            , message_serializer_( my_id_ )
            , socket_ipv4_{ message_socket_type::ipv4( io_service_, ipv4 ) }
            , socket_ipv6_{ message_socket_type::ipv6( io_service_, ipv6 ) }
            , routing_table_{ my_id_ }
            , value_store_{}
            , is_connected_{}
            , pending_tasks_{}
    {
        start_message_reception();
        discover_neighbors( initial_peer );

        LOG_DEBUG( engine, this ) << "created at '"
                << socket_ipv4_.local_endpoint() << "' and '"
                << socket_ipv6_.local_endpoint() << "' boostraping using peer '"
                << initial_peer << "'." << std::endl;
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

            auto c = create_store_value_context( id( key )
                                               , data
                                               , std::forward< HandlerType >( handler ) );
            store_value( c );
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

            auto c = create_find_value_context( id( key )
                                              , std::forward< HandlerType >( handler ) );
            find_value( c );
        }
    }

private:
    ///
    using endpoint_type = ip_endpoint;

    ///
    using routing_table_type = routing_table< endpoint_type >;

    ///
    using pending_task_type = std::function< void ( void ) >;

private:
    /**
     *
     */
    void
    start_message_reception
        ( void )
    {
        schedule_receive_on_socket( socket_ipv4_ );
        schedule_receive_on_socket( socket_ipv6_ );
    }

    /**
     *
     */
    void
    schedule_receive_on_socket
        ( message_socket_type & current_subnet )
    {
        auto on_new_message = [ this, &current_subnet ]
            ( std::error_code const& failure
            , endpoint_type const& sender
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            // Reception failure are fatal.
            if ( failure )
                throw std::system_error{ failure };

            handle_new_message( sender, i, e );
            schedule_receive_on_socket( current_subnet );
        };

        current_subnet.async_receive( on_new_message );
    }

    /**
     *
     */
    message_socket_type &
    get_socket_for
        ( endpoint_type const& e )
    {
        if ( e.address_.is_v4() )
            return socket_ipv4_;

        return socket_ipv6_;
    }

    /**
     *
     */
    template< typename HandlerType >
    std::shared_ptr< find_value_context< typename std::remove_reference< HandlerType >::type
                                       , data_type > >
    create_find_value_context
        ( id const& key
        , HandlerType && load_handler )
    {
        LOG_DEBUG( engine, this ) << "create find value context for '"
                << key << "' value." << std::endl;

        using handler_type = typename std::remove_reference< HandlerType >::type;
        using context = find_value_context< handler_type, data_type >;

        auto i = routing_table_.find( key ), e = routing_table_.end();
        return std::make_shared< context >( key, i, e
                                          , std::forward< HandlerType >( load_handler ) );
    }

    /**
     *
     */
    std::shared_ptr< notify_peer_context >
    create_notify_peer_context
        ( id const& key )
    {
        LOG_DEBUG( engine, this ) << "create find peer context for '"
                << key << "' peer." << std::endl;

        using context = notify_peer_context;

        auto i = routing_table_.find( key ), e = routing_table_.end();
        return std::make_shared< context >( key, i, e );
    }

    /**
     *
     */
    template< typename HandlerType >
    std::shared_ptr< store_value_context< typename std::remove_reference< HandlerType >::type
                                        , data_type > >
    create_store_value_context
        ( id const& key
        , data_type const& data
        , HandlerType && save_handler )
    {
        LOG_DEBUG( engine, this ) << "create store value context for '"
                << key << "' value(" << to_string( data )
                << ")." << std::endl;

        using handler_type = typename std::remove_reference< HandlerType >::type;
        using context = store_value_context< handler_type, data_type >;

        auto i = routing_table_.find( key ), e = routing_table_.end();
        assert( "at least one peer is reported" && i != e );
        return std::make_shared< context >( key, data, i, e
                                          , std::forward< HandlerType >( save_handler ) );
    }

    /**
     *
     */
    void
    handle_new_message
        ( endpoint_type const& sender
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
                response_router_.handle_new_response( sender, h, i, e );
                break;
        }
    }

    /**
     *
     */
    void
    add_peer_to_routing_table
        ( id const& peer_id
        , endpoint_type const& peer_endpoint )
    {
        LOG_DEBUG( engine, this ) << "adding '"
                << peer_id << "@" << peer_endpoint
                << "'." << std::endl;

        routing_table_.push( peer_id, peer_endpoint );
    }

    /**
     *
     */
    void
    handle_ping_request
        ( endpoint_type const& sender
        , header const& h )
    {
        LOG_DEBUG( engine, this ) << "handling ping request." << std::endl;

        send_response( h.random_token_
                     , header::PING_RESPONSE
                     , sender );
    }

    /**
     *
     */
    void
    handle_store_request
        ( endpoint_type const& sender
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
        ( endpoint_type const& sender
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
        ( endpoint_type const& sender
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
        send_response( random_token, response, sender );
    }

    /**
     *
     */
    void
    handle_find_value_request
        ( endpoint_type const& sender
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
            send_response( h.random_token_
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
        auto endoints_to_query
                = message_socket_type::resolve_endpoint( io_service_
                                                       , initial_peer );

        search_ourselves( std::move( endoints_to_query ) );
    }

    /**
     *
     */
    template< typename ResolvedEndpointType >
    void
    search_ourselves
        ( ResolvedEndpointType endpoints_to_query )
    {
        if ( endpoints_to_query.empty() )
            throw std::system_error
                    { make_error_code( INITIAL_PEER_FAILED_TO_RESPOND ) };

        // Retrieve the next endpoint to query.
        auto const endpoint_to_query = endpoints_to_query.back();
        endpoints_to_query.pop_back();

        // On message received, process it.
        auto on_message_received = [ this ]
            ( endpoint_type const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        { handle_initial_contact_response( s, h, i, e ); };

        // On error, retry with another endpoint.
        auto on_error = [ this, endpoints_to_query ]
            ( std::error_code const& )
        { search_ourselves( endpoints_to_query ); };

        send_request( id{ random_engine_ }
                    , find_peer_request_body{ my_id_ }
                    , endpoint_to_query
                    , INITIAL_CONTACT_RECEIVE_TIMEOUT
                    , on_message_received
                    , on_error );
    }

    /**
     *
     */
    void
    handle_initial_contact_response
        ( endpoint_type const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    {
        LOG_DEBUG( engine, this ) << "handling init contact response."
                << std::endl;

        if ( h.type_ != header::FIND_PEER_RESPONSE )
            return ;

        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize find peer response ("
                    << failure.message() << ")" << std::endl;

            return;
        }

        // Add discovered peers.
        for ( auto const& peer : response.peers_ )
            add_peer_to_routing_table( peer.id_, peer.endpoint_ );

        notify_neighbors();

        LOG_DEBUG( engine, this ) << "added '" << response.peers_.size()
                << "' initial peer(s)." << std::endl;
    }

    /**
     *  Refresh each bucket.
     */
    void
    notify_neighbors
        ( void )
    {
        id refresh_id = my_id_;

        for ( std::size_t i = id::BIT_SIZE; i > 0; -- i)
        {
            // Flip bit to select find peers in the current k_bucket.
            id::reference bit = refresh_id[ i - 1 ];
            bit = ! bit;

            auto c = create_notify_peer_context( refresh_id );

            notify_neighbors( c );
        }
    }

    /**
     *
     */
    void
    notify_neighbors
        ( std::shared_ptr< notify_peer_context > context )
    {
        LOG_DEBUG( engine, this ) << "sending find peer to notify '"
                << context->get_key() << "' owner bucket." << std::endl;

        find_peer_request_body const request{ context->get_key() };

        auto const closest_peers = context->select_new_closest_candidates
                ( CONCURRENT_FIND_PEER_REQUESTS_COUNT );

        for ( auto const& c : closest_peers )
            send_notify_peer_request( request, c, context );
    }

    /**
     *
     */
    void
    send_notify_peer_request
        ( find_peer_request_body const& request
        , peer const& current_peer
        , std::shared_ptr< notify_peer_context > context )
    {
        LOG_DEBUG( engine, this ) << "sending find peer to notify to '"
                << current_peer << "'." << std::endl;

        auto on_message_received = [ this, context, current_peer ]
            ( endpoint_type const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            context->flag_candidate_as_valid( current_peer.id_ );
            handle_notify_peer_response( s, h, i, e, context );
        };

        auto on_error = [ this, context, current_peer ]
            ( std::error_code const& )
        { context->flag_candidate_as_invalid( current_peer.id_ ); };

        send_request( id{ random_engine_ }
                    , request
                    , current_peer.endpoint_
                    , PEER_LOOKUP_TIMEOUT
                    , on_message_received
                    , on_error );
    }

    /**
     *
     */
    void
    handle_notify_peer_response
        ( endpoint_type const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< notify_peer_context > context )
    {
        LOG_DEBUG( engine, this ) << "handle notify peer response from '"
                << s << "'." << std::endl;

        assert( h.type_ == header::FIND_PEER_RESPONSE );
        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize find peer response ("
                    << failure.message() << ")" << std::endl;
            return;
        }

        // If new candidate have been discovered, ask them.
        if ( context->are_these_candidates_closest( response.peers_ ) )
            notify_neighbors( context );
    }
    /**
     *
     */
    template< typename HandlerType >
    void
    store_value
        ( std::shared_ptr< store_value_context< HandlerType, data_type > > context
        , std::size_t concurrent_requests_count = CONCURRENT_FIND_PEER_REQUESTS_COUNT )
    {
        LOG_DEBUG( engine, this ) << "sending find peer to store '"
                << context->get_key() << "' value." << std::endl;

        find_peer_request_body const request{ context->get_key() };

        auto const closest_candidates = context->select_new_closest_candidates
                ( concurrent_requests_count );

        assert( "at least one candidate exists" && ! closest_candidates.empty() );

        for ( auto const& c : closest_candidates )
            send_find_peer_to_store_request( request, c, context );
    }

    /**
     *
     */
    template< typename HandlerType >
    void
    send_find_peer_to_store_request
        ( find_peer_request_body const& request
        , peer const& current_candidate
        , std::shared_ptr< store_value_context< HandlerType, data_type > > context )
    {
        LOG_DEBUG( engine, this ) << "sending find peer request to store to '"
                << current_candidate << "'." << std::endl;

        // On message received, process it.
        auto on_message_received = [ this, context, current_candidate ]
            ( endpoint_type const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            context->flag_candidate_as_valid( current_candidate.id_ );

            handle_find_peer_to_store_response( s, h, i, e, context );
        };

        // On error, retry with another endpoint.
        auto on_error = [ this, context, current_candidate ]
            ( std::error_code const& )
        {
            // XXX: Can also flag candidate as invalid is
            // present in routing table.
            context->flag_candidate_as_invalid( current_candidate.id_ );

            // If no more requests are in flight
            // we know the closest peers hence ask
            // them to store the value.
            if ( context->have_all_requests_completed() )
                send_store_requests( context );
        };

        send_request( id{ random_engine_ }
                    , request
                    , current_candidate.endpoint_
                    , PEER_LOOKUP_TIMEOUT
                    , on_message_received
                    , on_error );
    }

    /**
     *
     */
    template< typename HandlerType >
    void
    find_value
        ( std::shared_ptr< find_value_context< HandlerType, data_type > > context )
    {
        find_value_request_body const request{ context->get_key() };

        auto const closest_candidates = context->select_new_closest_candidates
                ( CONCURRENT_FIND_PEER_REQUESTS_COUNT );

        assert( "at least one candidate exists" && ! closest_candidates.empty() );

        for ( auto const& c : closest_candidates )
            send_find_value_request( request, c, context );
    }

    /**
     *
     */
    template< typename HandlerType >
    void
    send_find_value_request
        ( find_value_request_body const& request
        , peer const& current_candidate
        , std::shared_ptr< find_value_context< HandlerType, data_type > > context )
    {
        LOG_DEBUG( engine, this ) << "sending find '" << context->get_key()
                << "' value request to '"
                << current_candidate << "'." << std::endl;

        // On message received, process it.
        auto on_message_received = [ this, context, current_candidate ]
            ( endpoint_type const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            if ( context->is_caller_notified() )
                return;

            context->flag_candidate_as_valid( current_candidate.id_ );
            handle_find_value_response( s, h, i, e, context );
        };

        // On error, retry with another endpoint.
        auto on_error = [ this, context, current_candidate ]
            ( std::error_code const& )
        {
            if ( context->is_caller_notified() )
                return;

            // XXX: Current current_candidate must be flagged as stale.
            context->flag_candidate_as_invalid( current_candidate.id_ );
            find_value( context );
        };

        send_request( id{ random_engine_ }
                    , request
                    , current_candidate.endpoint_
                    , PEER_LOOKUP_TIMEOUT
                    , on_message_received
                    , on_error );
    }

    /**
     *  @brief This method is called while searching for
     *         the peer owner of the value.
     */
    template< typename HandlerType >
    void
    handle_find_value_response
        ( endpoint_type const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< find_value_context< HandlerType, data_type > > context )
    {
        LOG_DEBUG( engine, this ) << "handling response to find '"
                << context->get_key() << "' value." << std::endl;

        if ( h.type_ == header::FIND_PEER_RESPONSE )
            // The current peer didn't know the value
            // but provided closest peers.
            send_find_value_requests_on_closer_peers( i, e, context );
        else if ( h.type_ == header::FIND_VALUE_RESPONSE )
            // The current peer knows the value.
            process_found_value( i, e, context );
    }

    /**
     *  @brief This method is called when closest peers
     *         to the value we are looking are discovered.
     *         It recursively query new discovered peers
     *         or report an error to the use handler if
     *         all peers have been tried.
     */
    template< typename HandlerType >
    void
    send_find_value_requests_on_closer_peers
        ( buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< find_value_context< HandlerType, data_type > > context )
    {
        LOG_DEBUG( engine, this ) << "checking if found closest peers to '"
                << context->get_key() << "' value from closer peers."
                << std::endl;

        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, this ) << "failed to deserialize find peer response '"
                    << context->get_key() << "' because ("
                    << failure.message() << ")." << std::endl;

            return;
        }

        if ( context->are_these_candidates_closest( response.peers_ ) )
            find_value( context );

        if ( context->have_all_requests_completed() )
            context->notify_caller( make_error_code( VALUE_NOT_FOUND ) );
    }

    /**
     *  @brief This method is called once the searched value
     *         has been found. It forwards the value to
     *         the user handler.
     */
    template< typename HandlerType >
    void
    process_found_value
        ( buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< find_value_context< HandlerType, data_type > > context )
    {
        LOG_DEBUG( engine, this ) << "found '" << context->get_key()
                << "' value." << std::endl;

        find_value_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize find value response ("
                    << failure.message() << ")" << std::endl;
            return;
        }

        context->notify_caller( response.data_ );
    }

    /**
     *
     */
    template< typename HandlerType >
    void
    handle_find_peer_to_store_response
        ( endpoint_type const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< store_value_context< HandlerType, data_type > > context )
    {
        LOG_DEBUG( engine, this ) << "handle find peer to store response from '"
                << s << "'." << std::endl;

        assert( h.type_ == header::FIND_PEER_RESPONSE );
        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, this )
                    << "failed to deserialize find peer response ("
                    << failure.message() << ")" << std::endl;
            return;
        }

        // If new candidate have been discovered, ask them.
        if ( context->are_these_candidates_closest( response.peers_ ) )
            store_value( context );
        else
        {
            LOG_DEBUG( engine, this ) << "'" << s
                    << "' did'nt provided closer peer to '"
                    << context->get_key() << "' value." << std::endl;

            // Else if all candidates have responded,
            // we know the closest peers hence ask them
            // to store the value.
            if ( context->have_all_requests_completed() )
                send_store_requests( context );
            else
                LOG_DEBUG( engine, this )
                        << "waiting for other peer(s) response to find '"
                        << context->get_key() << "' value." << std::endl;
        }
    }

    /**
     *
     */
    template< typename HandlerType >
    void
    send_store_requests
        ( std::shared_ptr< store_value_context< HandlerType, data_type > > context )
    {
        auto const & candidates
                = context->select_closest_valid_candidates( REDUNDANT_SAVE_COUNT );

        assert( "at least one candidate exists" && ! candidates.empty() );

        for ( auto c : candidates )
            send_store_request( c, context );

        context->notify_caller( std::error_code{} );
    }

    /**
     *
     */
    template< typename HandlerType >
    void
    send_store_request
        ( peer const& current_candidate
        , std::shared_ptr< store_value_context< HandlerType, data_type > > context )
    {
        LOG_DEBUG( engine, this ) << "send store request of '"
                << context->get_key() << "' to '"
                << current_candidate << "'." << std::endl;

        store_value_request_body const request{ context->get_key()
                                              , context->get_data() };
        send_request( id{ random_engine_ }
                    , request
                    , current_candidate.endpoint_ );
    }

    /**
     *
     */
    void
    handle_new_message
        ( endpoint_type const& sender
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

        add_peer_to_routing_table( h.source_id_, sender );

        handle_new_message( sender, h, i, e );

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
    template< typename Request, typename OnResponseReceived, typename OnError >
    void
    send_request
        ( id const& response_id
        , Request const& request
        , endpoint_type const& e
        , timer::duration const& timeout
        , OnResponseReceived const& on_response_received
        , OnError const& on_error )
    {
        // Generate the request buffer.
        auto message = message_serializer_.serialize( request, response_id );

        // This lamba will keep the request message alive.
        auto on_request_sent = [ this, response_id
                               , on_response_received, on_error
                               , timeout ]
            ( std::error_code const& failure )
        {
            if ( failure )
                on_error( failure );
            else
                response_router_. register_temporary_association( response_id, timeout
                                                                , on_response_received
                                                                , on_error );
        };

        // Serialize the request and send it.
        get_socket_for( e ).async_send( message, e, on_request_sent );
    }

    /**
     *
     */
    template< typename Request >
    void
    send_request
        ( id const& response_id
        , Request const& request
        , endpoint_type const& e )
    { send_response( response_id, request, e ); }

    /**
     *
     */
    template< typename Response >
    void
    send_response
        ( id const& response_id
        , Response const& response
        , endpoint_type const& e )
    {
        auto message = message_serializer_.serialize( response, response_id );

        auto on_response_sent = []
            ( std::error_code const& /* failure */ )
        { };

        // Serialize the message and send it.
        get_socket_for( e ).async_send( message, e, on_response_sent );
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
#if 0
            io_service_.post( std::move( pending_tasks_.front() ) );
#else
            pending_tasks_.front()();
#endif
            pending_tasks_.pop();
        }
    }

private:
    ///
    std::default_random_engine random_engine_;
    ///
    id my_id_;
    ///
    boost::asio::io_service & io_service_;
    ///
    response_router response_router_;
    ///
    message_serializer message_serializer_;
    ///
    message_socket_type socket_ipv4_;
    ///
    message_socket_type socket_ipv6_;
    ///
    routing_table_type routing_table_;
    ///
    value_store< id, data_type > value_store_;
    ///
    bool is_connected_;
    ///
    std::queue< pending_task_type > pending_tasks_;
};

} // namespace detail
} // namespace kademlia

#endif
