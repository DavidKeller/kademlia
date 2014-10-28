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

#ifndef KADEMLIA_FIND_VALUE_CONTEXT_HPP
#define KADEMLIA_FIND_VALUE_CONTEXT_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <system_error>
#include <memory>
#include <type_traits>

#include <kademlia/error.hpp>

#include "kademlia/value_context.hpp"
#include "kademlia/log.hpp"
#include "kademlia/constants.hpp"
#include "kademlia/message.hpp"

namespace kademlia {
namespace detail {

///
template< typename LoadHandlerType, typename CoreType, typename DataType >
class find_value_context final
    : public value_context
{
public:
    ///
    using load_handler_type = LoadHandlerType;

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
        , core_type & core
        , RoutingTableType & routing_table
        , load_handler_type handler )
    {
        std::shared_ptr< find_value_context > c;
        c.reset( new find_value_context( key
                                       , core
                                       , routing_table
                                       , std::move( handler ) ) );

        find_value( c );
    }

private:
    /**
     *
     */
    template< typename RoutingTableType >
    find_value_context
        ( id const & searched_key
        , core_type & core
        , RoutingTableType & routing_table
        , load_handler_type load_handler )
            : value_context( searched_key
                           , routing_table.find( searched_key )
                           , routing_table.end() )
            , core_( core )
            , load_handler_( std::move( load_handler ) )
            , is_finished_()
    {
        LOG_DEBUG( find_value_context, this )
                << "create find value context for '"
                << searched_key << "' value." << std::endl;
    }

    /**
     *
     */
    void
    notify_caller
        ( data_type const& data )
    {
        load_handler_( std::error_code(), data );
        is_finished_ = true;
    }

    /**
     *
     */
    void
    notify_caller
        ( std::error_code const& failure )
    {
        load_handler_( failure, data_type{} );
        is_finished_ = true;
    }

    /**
     *
     */
    bool
    is_caller_notified
        ( void )
        const
    { return is_finished_; }

    /**
     *
     */
    static void
    find_value
        ( std::shared_ptr< find_value_context > context )
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
    static void
    send_find_value_request
        ( find_value_request_body const& request
        , peer const& current_candidate
        , std::shared_ptr< find_value_context > context )
    {
        LOG_DEBUG( find_value_context, context.get() ) << "sending find '" << context->get_key()
                << "' value request to '"
                << current_candidate << "'." << std::endl;

        // On message received, process it.
        auto on_message_received = [ context, current_candidate ]
            ( ip_endpoint const& s
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
        auto on_error = [ context, current_candidate ]
            ( std::error_code const& )
        {
            if ( context->is_caller_notified() )
                return;

            // XXX: Current current_candidate must be flagged as stale.
            context->flag_candidate_as_invalid( current_candidate.id_ );
            find_value( context );
        };

        context->core_.send_request( request
                                   , current_candidate.endpoint_
                                   , PEER_LOOKUP_TIMEOUT
                                   , on_message_received
                                   , on_error );
    }

    /**
     *  @brief This method is called while searching for
     *         the peer owner of the value.
     */
    static void
    handle_find_value_response
        ( ip_endpoint const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< find_value_context > context )
    {
        LOG_DEBUG( engine, context.get() ) << "handling response to find '"
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
    static void
    send_find_value_requests_on_closer_peers
        ( buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< find_value_context > context )
    {
        LOG_DEBUG( engine, context.get() ) << "checking if found closest peers to '"
                << context->get_key() << "' value from closer peers."
                << std::endl;

        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, context.get() ) << "failed to deserialize find peer response '"
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
    static void
    process_found_value
        ( buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< find_value_context > context )
    {
        LOG_DEBUG( engine, context.get() ) << "found '" << context->get_key()
                << "' value." << std::endl;

        find_value_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( engine, context.get() )
                    << "failed to deserialize find value response ("
                    << failure.message() << ")" << std::endl;
            return;
        }

        context->notify_caller( response.data_ );
    }

private:
    ///
    core_type & core_;
    ///
    load_handler_type load_handler_;
    ///
    bool is_finished_;
};

/**
 *
 */
template< typename DataType
        , typename CoreType
        , typename RoutingTableType
        , typename HandlerType >
void
start_find_value_task
    ( id const& key
    , CoreType & core
    , RoutingTableType & routing_table
    , HandlerType && handler )
{
    using handler_type = typename std::remove_reference< HandlerType >::type;
    using context = find_value_context< handler_type, CoreType, DataType >;

    context::start( key, core, routing_table
                  , std::forward< HandlerType >( handler ) );
}

} // namespace detail
} // namespace kademlia

#endif

