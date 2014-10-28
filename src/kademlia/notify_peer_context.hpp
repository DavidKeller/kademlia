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

#ifndef KADEMLIA_NOTIFY_PEER_CONTEXT_HPP
#define KADEMLIA_NOTIFY_PEER_CONTEXT_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <memory>
#include <system_error>

#include "kademlia/value_context.hpp"
#include "kademlia/message.hpp"
#include "kademlia/core.hpp"

namespace kademlia {
namespace detail {

///
template< typename CoreType >
class notify_peer_context final
    : public value_context
{
public:
    ///
    using core_type = CoreType;

    ///
    using endpoint_type = typename core_type::endpoint_type;

public:
    /**
     *
     */
    template< typename RoutingTableType >
    static void
    start
        ( detail::id const & key
        , core_type & core
        , RoutingTableType & routing_table )
    {
        std::shared_ptr< notify_peer_context > c;
        c.reset( new notify_peer_context( key, core, routing_table ) );

        notify_neighbors( c );
    }

private:
    /**
     *
     */
    template< typename RoutingTableType >
    notify_peer_context
        ( detail::id const & key
        , core_type & core
        , RoutingTableType & routing_table )
            : value_context( key
                           , routing_table.find( key )
                           , routing_table.end() )
            , core_( core )
    {
        LOG_DEBUG( notify_peer_context, this )
                << "create find peer context for '"
                << key << "' peer." << std::endl;
    }

    /**
     *
     */
    static void
    notify_neighbors
        ( std::shared_ptr< notify_peer_context > context )
    {
        LOG_DEBUG( notify_peer_context, context.get() )
                << "sending find peer to notify '"
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
    static void
    send_notify_peer_request
        ( find_peer_request_body const& request
        , peer const& current_peer
        , std::shared_ptr< notify_peer_context > context )
    {
        LOG_DEBUG( notify_peer_context, context.get() )
                << "sending find peer to notify to '"
                << current_peer << "'." << std::endl;

        auto on_message_received = [ context, current_peer ]
            ( endpoint_type const& s
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e )
        {
            context->flag_candidate_as_valid( current_peer.id_ );
            handle_notify_peer_response( s, h, i, e, context );
        };

        auto on_error = [ context, current_peer ]
            ( std::error_code const& )
        { context->flag_candidate_as_invalid( current_peer.id_ ); };

        context->core_.send_request( request
                                   , current_peer.endpoint_
                                   , PEER_LOOKUP_TIMEOUT
                                   , on_message_received
                                   , on_error );
    }

    /**
     *
     */
    static void
    handle_notify_peer_response
        ( endpoint_type const& s
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e
        , std::shared_ptr< notify_peer_context > context )
    {
        LOG_DEBUG( notify_peer_context, context.get() )
                << "handle notify peer response from '" << s
                << "'." << std::endl;

        assert( h.type_ == header::FIND_PEER_RESPONSE );
        find_peer_response_body response;
        if ( auto failure = deserialize( i, e, response ) )
        {
            LOG_DEBUG( notify_peer_context, &context )
                    << "failed to deserialize find peer response ("
                    << failure.message() << ")" << std::endl;
            return;
        }

        // If new candidate have been discovered, ask them.
        if ( context->are_these_candidates_closest( response.peers_ ) )
            notify_neighbors( context );
    }

private:
    ///
    core_type & core_;
};

/**
 *
 */
template< typename CoreType, typename RoutingTableType >
void
start_notify_peer_task
    ( id const& key
    , CoreType & core
    , RoutingTableType & routing_table )
{
    using context = notify_peer_context< CoreType >;

    context::start( key, core, routing_table );
}

} // namespace detail
} // namespace kademlia

#endif

