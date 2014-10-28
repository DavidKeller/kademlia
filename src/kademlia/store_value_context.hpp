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

#ifndef KADEMLIA_STORE_VALUE_CONTEXT_HPP
#define KADEMLIA_STORE_VALUE_CONTEXT_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <memory>
#include <type_traits>
#include <system_error>

#include "kademlia/value_context.hpp"
#include "kademlia/log.hpp"
#include "kademlia/message.hpp"
#include "kademlia/constants.hpp"

namespace kademlia {
namespace detail {

///
template< typename SaveHandlerType, typename CoreType, typename DataType >
class store_value_context final
    : public value_context
{
public:
    ///
    using save_handler_type = SaveHandlerType;

    ///
    using core_type = CoreType;

    ///
    using data_type = DataType;

public:
    /**
     *
     */
    template< typename RoutingTableType >
    static void
    start
        ( detail::id const & key
        , data_type const& data
        , core_type & core
        , RoutingTableType & routing_table
        , save_handler_type handler )
    {
        std::shared_ptr< store_value_context > c;
        c.reset( new store_value_context( key
                                        , data
                                        , core
                                        , routing_table
                                        , std::move( handler ) ) );

        store_value( c );
    }

private:
    /**
     *
     */
    template< typename RoutingTableType, typename HandlerType >
    store_value_context
        ( detail::id const & key
        , data_type const& data
        , core_type & core
        , RoutingTableType & routing_table
        , HandlerType && save_handler )
            : value_context( key
                           , routing_table.find( key )
                           , routing_table.end() )
            , core_( core )
            , data_( data )
            , save_handler_( std::forward< HandlerType >( save_handler ) )
    {
        LOG_DEBUG( store_value_context, this )
                << "create store value context for '"
                << key << "' value(" << to_string( data )
                << ")." << std::endl;
    }

    /**
     *
     */
    void
    notify_caller
        ( std::error_code const& failure )
    { save_handler_( failure ); }

    /**
     *
     */
    data_type const&
    get_data
        ( void )
        const
    { return data_; }

    /**
     *
     */
    static void
    store_value
        ( std::shared_ptr< store_value_context > context
        , std::size_t concurrent_requests_count = CONCURRENT_FIND_PEER_REQUESTS_COUNT )
    {
        LOG_DEBUG( engine, context.get() )
                << "sending find peer to store '"
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
    static void
    send_find_peer_to_store_request
        ( find_peer_request_body const& request
        , peer const& current_candidate
        , std::shared_ptr< store_value_context > context )
    {
        LOG_DEBUG( engine, context.get() )
                << "sending find peer request to store to '"
                << current_candidate << "'." << std::endl;

        // On message received, process it.
        auto on_message_received = [ context, current_candidate ]
            ( ip_endpoint const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            context->flag_candidate_as_valid( current_candidate.id_ );

            handle_find_peer_to_store_response( s, h, i, e, context );
        };

        // On error, retry with another endpoint.
        auto on_error = [ context, current_candidate ]
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

        context->core_.send_request( request
                                   , current_candidate.endpoint_
                                   , PEER_LOOKUP_TIMEOUT
                                   , on_message_received
                                   , on_error );
    }

    /**
     *
     */
    static void
    handle_find_peer_to_store_response
        ( ip_endpoint const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< store_value_context > context )
    {
        LOG_DEBUG( engine, context.get() )
                << "handle find peer to store response from '"
                << s << "'." << std::endl;

        assert( h.type_ == header::FIND_PEER_RESPONSE );
        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, context.get() )
                    << "failed to deserialize find peer response ("
                    << failure.message() << ")" << std::endl;
            return;
        }

        // If new candidate have been discovered, ask them.
        if ( context->are_these_candidates_closest( response.peers_ ) )
            store_value( context );
        else
        {
            LOG_DEBUG( engine, context.get() ) << "'" << s
                    << "' did'nt provided closer peer to '"
                    << context->get_key() << "' value." << std::endl;

            // Else if all candidates have responded,
            // we know the closest peers hence ask them
            // to store the value.
            if ( context->have_all_requests_completed() )
                send_store_requests( context );
            else
                LOG_DEBUG( engine, context.get() )
                        << "waiting for other peer(s) response to find '"
                        << context->get_key() << "' value." << std::endl;
        }
    }

    /**
     *
     */
    static void
    send_store_requests
        ( std::shared_ptr< store_value_context > context )
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
    static void
    send_store_request
        ( peer const& current_candidate
        , std::shared_ptr< store_value_context > context )
    {
        LOG_DEBUG( engine, context.get() )
                << "send store request of '"
                << context->get_key() << "' to '"
                << current_candidate << "'." << std::endl;

        store_value_request_body const request{ context->get_key()
                                              , context->get_data() };
        context->core_.send_request( request, current_candidate.endpoint_ );
    }

private:
    ///
    core_type & core_;
    ///
    data_type data_;
    ///
    save_handler_type save_handler_;
};

/**
 *
 */
template< typename DataType
        , typename CoreType
        , typename RoutingTableType
        , typename HandlerType >
void
start_store_value_task
    ( id const& key
    , DataType const& data
    , CoreType & core
    , RoutingTableType & routing_table
    , HandlerType && save_handler )
{
    using handler_type = typename std::remove_reference< HandlerType >::type;
    using context = store_value_context< handler_type, CoreType, DataType >;

    context::start( key, data, core, routing_table
                  , std::forward< HandlerType >( save_handler ) );
}

} // namespace detail
} // namespace kademlia

#endif

