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

#ifndef KADEMLIA_STORE_VALUE_TASK_HPP
#define KADEMLIA_STORE_VALUE_TASK_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <memory>
#include <type_traits>
#include <system_error>

#include "kademlia/lookup_task.hpp"
#include "kademlia/log.hpp"
#include "kademlia/message.hpp"
#include "kademlia/constants.hpp"

namespace kademlia {
namespace detail {

///
template< typename SaveHandlerType, typename TrackerType, typename DataType >
class store_value_task final
    : public lookup_task
{
public:
    ///
    using save_handler_type = SaveHandlerType;

    ///
    using tracker_type = TrackerType;

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
        , tracker_type & tracker
        , RoutingTableType & routing_table
        , save_handler_type handler )
    {
        std::shared_ptr< store_value_task > c;
        c.reset( new store_value_task( key
                                     , data
                                     , tracker
                                     , routing_table
                                     , std::move( handler ) ) );

        try_to_store_value( c );
    }

private:
    /**
     *
     */
    template< typename RoutingTableType, typename HandlerType >
    store_value_task
        ( detail::id const & key
        , data_type const& data
        , tracker_type & tracker
        , RoutingTableType & routing_table
        , HandlerType && save_handler )
            : lookup_task( key
                         , routing_table.find( key )
                         , routing_table.end() )
            , tracker_( tracker )
            , data_( data )
            , save_handler_( std::forward< HandlerType >( save_handler ) )
    {
        LOG_DEBUG( store_value_task, this )
                << "create store value task for '"
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
    try_to_store_value
        ( std::shared_ptr< store_value_task > task
        , std::size_t concurrent_requests_count = CONCURRENT_FIND_PEER_REQUESTS_COUNT )
    {
        LOG_DEBUG( store_value_task, task.get() )
                << "trying to find closer peer to store '"
                << task->get_key() << "' value." << std::endl;

        find_peer_request_body const request{ task->get_key() };

        auto const closest_candidates = task->select_new_closest_candidates
                ( concurrent_requests_count );

        for ( auto const& c : closest_candidates )
            send_find_peer_to_store_request( request, c, task );

        // If no more requests are in flight
        // we know the closest peers hence ask
        // them to store the value.
        if ( task->have_all_requests_completed() )
            send_store_requests( task ); 
    }

    /**
     *
     */
    static void
    send_find_peer_to_store_request
        ( find_peer_request_body const& request
        , peer const& current_candidate
        , std::shared_ptr< store_value_task > task )
    {
        LOG_DEBUG( store_value_task, task.get() )
                << "sending find peer request to store '"
                << task->get_key() << "' to '"
                << current_candidate << "'." << std::endl;

        // On message received, process it.
        auto on_message_received = [ task ]
            ( ip_endpoint const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            handle_find_peer_to_store_response( s, h, i, e, task );
        };

        // On error, retry with another endpoint.
        auto on_error = [ task, current_candidate ]
            ( std::error_code const& )
        {
            // XXX: Can also flag candidate as invalid is
            // present in routing table.
            task->flag_candidate_as_invalid( current_candidate.id_ );

            try_to_store_value( task );
        };

        task->tracker_.send_request( request
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
        , std::shared_ptr< store_value_task > task )
    {
        LOG_DEBUG( store_value_task, task.get() )
                << "handle find peer to store response from '"
                << s << "'." << std::endl;

        if ( h.type_ != header::FIND_PEER_RESPONSE )
        {
            LOG_DEBUG( store_value_task, task.get() )
                    << "unexpected find peer response (type="
                    << int( h.type_ ) << ")" << std::endl;

            task->flag_candidate_as_invalid( h.source_id_ );
            try_to_store_value( task );
            return;

        };

        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( store_value_task, task.get() )
                    << "failed to deserialize find peer response ("
                    << failure.message() << ")" << std::endl;
            task->flag_candidate_as_invalid( h.source_id_ );
        }
        else
        {
            task->flag_candidate_as_valid( h.source_id_ );
            task->add_candidates( response.peers_ );
        }

        try_to_store_value( task );
    }

    /**
     *
     */
    static void
    send_store_requests
        ( std::shared_ptr< store_value_task > task )
    {
        auto const & candidates
                = task->select_closest_valid_candidates( REDUNDANT_SAVE_COUNT );

        for ( auto c : candidates )
            send_store_request( c, task );

        if ( candidates.empty() )
            task->notify_caller( make_error_code( INITIAL_PEER_FAILED_TO_RESPOND ) );
        else
            task->notify_caller( std::error_code{} );
    }

    /**
     *
     */
    static void
    send_store_request
        ( peer const& current_candidate
        , std::shared_ptr< store_value_task > task )
    {
        LOG_DEBUG( store_value_task, task.get() )
                << "send store request of '"
                << task->get_key() << "' to '"
                << current_candidate << "'." << std::endl;

        store_value_request_body const request{ task->get_key()
                                              , task->get_data() };
        task->tracker_.send_request( request, current_candidate.endpoint_ );
    }

private:
    ///
    tracker_type & tracker_;
    ///
    data_type data_;
    ///
    save_handler_type save_handler_;
};

/**
 *
 */
template< typename DataType
        , typename TrackerType
        , typename RoutingTableType
        , typename HandlerType >
void
start_store_value_task
    ( id const& key
    , DataType const& data
    , TrackerType & tracker
    , RoutingTableType & routing_table
    , HandlerType && save_handler )
{
    using handler_type = typename std::decay< HandlerType >::type;
    using task = store_value_task< handler_type, TrackerType, DataType >;

    task::start( key, data, tracker, routing_table
               , std::forward< HandlerType >( save_handler ) );
}

} // namespace detail
} // namespace kademlia

#endif

