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

#ifndef KADEMLIA_DISCOVER_NEIGHBORS_TASK_HPP
#define KADEMLIA_DISCOVER_NEIGHBORS_TASK_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <system_error>
#include <memory>
#include <type_traits>

#include <kademlia/error.hpp>

#include "kademlia/log.hpp"
#include "kademlia/constants.hpp"
#include "kademlia/message.hpp"
#include "kademlia/notify_peer_task.hpp"

namespace kademlia {
namespace detail {

///
template< typename TrackerType
        , typename RoutingTableType
        , typename EndpointsType >
class discover_neighbors_task final
{
public:
    ///
    using tracker_type = TrackerType;
    ///
    using endpoints_type = EndpointsType;
    ///
    using routing_table_type = RoutingTableType;

public:
    /**
     *
     */
    static void
    start
        ( id const & my_id
        , tracker_type & tracker
        , routing_table_type & routing_table
        , endpoints_type && endpoints_to_query )
    {
        std::shared_ptr< discover_neighbors_task > d;
        d.reset( new discover_neighbors_task( my_id
                                            , tracker
                                            , routing_table
                                            , std::move( endpoints_to_query ) ) );

        search_ourselves( d );
    }

private:
    /**
     *
     */
    discover_neighbors_task
        ( id const & my_id
        , tracker_type & tracker
        , routing_table_type & routing_table
        , endpoints_type && endpoints_to_query )
            : my_id_( my_id )
            , tracker_( tracker )
            , routing_table_( routing_table )
            , endpoints_to_query_( std::move( endpoints_to_query ) )
    {
        LOG_DEBUG( discover_neighbors_task, this )
                << "create discover neighbors task." << std::endl;
    }

    /**
     *
     */
    static void
    search_ourselves
        ( std::shared_ptr< discover_neighbors_task > task )
    {
        if ( task->endpoints_to_query_.empty() )
            throw std::system_error
                    { make_error_code( INITIAL_PEER_FAILED_TO_RESPOND ) };

        // Retrieve the next endpoint to query.
        auto const endpoint_to_query = task->endpoints_to_query_.back();
        task->endpoints_to_query_.pop_back();

        // On message received, process it.
        auto on_message_received = [ task ]
            ( ip_endpoint const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        { task->handle_initial_contact_response( s, h, i, e ); };

        // On error, retry with another endpoint.
        auto on_error = [ task ]
            ( std::error_code const& )
        { search_ourselves( task ); };

        task->tracker_.send_request( find_peer_request_body{ task->my_id_ }
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
        ( ip_endpoint const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e )
    {
        LOG_DEBUG( discover_neighbors_task, this )
                << "handling initial contact response."
                << std::endl;

        if ( h.type_ != header::FIND_PEER_RESPONSE )
            return;

        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( discover_neighbors_task, this )
                    << "failed to deserialize find peer response ("
                    << failure.message() << ")" << std::endl;

            return;
        }

        // Add discovered peers.
        for ( auto const& peer : response.peers_ )
            routing_table_.push( peer.id_, peer.endpoint_ );

        LOG_DEBUG( discover_neighbors_task, this )
                << "added '" << response.peers_.size()
                << "' initial peer(s)." << std::endl;

        notify_neighbors();
    }

    /**
     *  Refresh each bucket.
     */
    void
    notify_neighbors
        ( void )
    {
        id refresh_id = my_id_;

        for ( std::size_t i = id::BIT_SIZE; i > 0; -- i )
        {
            // Flip bit to select find peers in the current k_bucket.
            id::reference bit = refresh_id[ i - 1 ];
            bit = ! bit;

            start_notify_peer_task( refresh_id, tracker_, routing_table_ );
        }
    }

private:
    ///
    id const& my_id_;
    ///
    tracker_type & tracker_;
    ///
    routing_table_type & routing_table_;
    ///
    endpoints_type endpoints_to_query_;
};

/**
 *
 */
template< typename TrackerType
        , typename RoutingTableType
        , typename EndpointsType >
void
start_discover_neighbors_task
    ( id const& my_id
    , TrackerType & tracker
    , RoutingTableType & routing_table
    , EndpointsType && endpoints_to_query )
{
    using endpoints_type = typename std::remove_reference< EndpointsType >::type;
    using task = discover_neighbors_task< TrackerType
                                        , RoutingTableType
                                        , endpoints_type >;

    task::start( my_id, tracker, routing_table
               , std::forward< EndpointsType >( endpoints_to_query ) );
}

} // namespace detail
} // namespace kademlia

#endif

