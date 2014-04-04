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

#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <random>
#include <memory>
#include <boost/asio/io_service.hpp>

#include <kademlia/error.hpp>

#include "message_socket.hpp"
#include "message.hpp"
#include "routing_table.hpp"
#include "subnet.hpp"
#include "response_dispatcher.hpp"
#include "timer.hpp"
#include "value_store.hpp"

namespace kademlia {

namespace {

// k 
CXX11_CONSTEXPR std::size_t ROUTING_TABLE_BUCKET_SIZE{ 20 };
// a
CXX11_CONSTEXPR std::size_t CONCURRENT_FIND_NODE_REQUESTS_COUNT{ 3 };

CXX11_CONSTEXPR std::chrono::milliseconds INITIAL_CONTACT_RECEIVE_TIMEOUT{ 1000 };
CXX11_CONSTEXPR std::chrono::milliseconds NODE_LOOKUP_TIMEOUT{ 20 };

} // anonymous namespace

/**
 *
 */
class session::impl final
{
public:
    /**
     *
     */
    explicit 
    impl
        ( endpoint const& initial_peer
        , endpoint const& listen_on_ipv4
        , endpoint const& listen_on_ipv6 )
            : random_engine_{ std::random_device{}() }
            , my_id_( random_engine_ )
            , io_service_{}
            , initial_peer_{ initial_peer }
            , ipv4_subnet_{ create_ipv4_subnet( io_service_, listen_on_ipv4 ) }
            , ipv6_subnet_{ create_ipv6_subnet( io_service_, listen_on_ipv6 ) }
            , routing_table_{ my_id_ }
            , response_dispatcher_{}
            , timer_{ io_service_ }
            , value_store_{}
            , main_failure_{}
    { }
    
    /**
     *
     */
    void
    async_save
        ( key_type const& key 
        , data_type const& data
        , save_handler_type handler )
    { 
        throw std::system_error{ make_error_code( UNIMPLEMENTED ) }; 
    }

    /**
     *
     */
    void
    async_load
        ( key_type const& key
        , load_handler_type handler )
    { 
        find_value( create_find_value_context( detail::id{ key }
                                             , handler ) );
    }

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
            io_service_.poll();
        
        io_service_.stop();

        return main_failure_;
    }

    /**
     *
     */
    void
    abort
        ( void )
    { 
        auto set_abort_flag = [ this ] ( void )
        { main_failure_ = make_error_code( RUN_ABORTED ); };

        io_service_.post( set_abort_flag );
    }

private:
    ///
    class find_value_context
    {
    public:
        template< typename Iterator >
        explicit
        find_value_context( detail::id const & searched_key
                          , Iterator i, Iterator e
                          , load_handler_type load_handler )
            : searched_key_{ searched_key }
            , in_flight_requests_count_{ 0 }
            , candidates_{}
            , load_handler_{ std::move( load_handler ) }
        { 
            for ( std::size_t n = ROUTING_TABLE_BUCKET_SIZE
                ; n > 0 && i != e
                ; ++i, --n )
                add_candidate( i->first, i->second );
        }

        void
        add_candidate
            ( detail::id const& i
            , detail::message_socket::endpoint_type const& e )
        {
            auto const distance = detail::distance( i, searched_key_ );
            candidates_.emplace( distance, candidate{ e, false } );
        }

        void
        ack_candidate_response
            ( void )
        { -- in_flight_requests_count_; }

        std::vector< detail::message_socket::endpoint_type >
        select_n_candidates
            ( std::size_t candidates_max_count )
        {
            std::vector< detail::message_socket::endpoint_type > candidates;

            for ( auto i = candidates_.begin(), e = candidates_.end()
                ; i != e && in_flight_requests_count_ < candidates_max_count 
                ; ++ i)
                if ( ! i->second.is_contacted_ )
                {
                    i->second.is_contacted_ = true;
                    ++ in_flight_requests_count_;
                    candidates.push_back( i->second.endpoint_ );
                }

            return std::move( candidates );
        }

        detail::message_socket::endpoint_type
        get_closest_candidate
            ( void )
            const
        { return candidates_.begin()->second.endpoint_; }

        bool
        have_all_requests_completed
            ( void )
            const
        { return in_flight_requests_count_ == 0; }
        
        detail::id
        get_searched_key
            ( void )
            const
        { return searched_key_; }

        void
        forward_to_client
            ( data_type const& data )
        { load_handler_( std::error_code{}, data ); }

        void
        forward_to_client
            ( std::error_code const& failure )
        { load_handler_( failure, data_type{} ); }

    private:
        struct candidate
        {
            detail::message_socket::endpoint_type endpoint_;
            bool is_contacted_;
        };

    private:
        detail::id searched_key_;
        std::size_t in_flight_requests_count_;
        std::map< detail::id, candidate > candidates_;
        load_handler_type load_handler_;
    };

private:
    /**
     *
     */
    void
    init
        ( void )
    {
        io_service_.reset();

        start_receive_on_each_subnet();
        discover_neighbors();
    }

    /**
     *
     */
    static detail::subnet
    create_ipv4_subnet
        ( boost::asio::io_service & io_service
        , endpoint const& ipv4_endpoint )
    {
        auto endpoints = detail::resolve_endpoint( io_service, ipv4_endpoint );

        for ( auto const& i : endpoints )
        {
            if ( i.address().is_v4() )
                return detail::subnet{ detail::create_socket( io_service, i ) };
        }

        throw std::system_error{ make_error_code( INVALID_IPV4_ADDRESS ) };
    }

    /**
     *
     */
    static detail::subnet
    create_ipv6_subnet
        ( boost::asio::io_service & io_service
        , endpoint const& ipv6_endpoint )
    {
        auto endpoints = detail::resolve_endpoint( io_service, ipv6_endpoint );

        for ( auto const& i : endpoints )
        {
            if ( i.address().is_v6() )
                return detail::subnet{ detail::create_socket( io_service, i ) };
        }

        throw std::system_error{ make_error_code( INVALID_IPV6_ADDRESS ) };
    }

    /**
     *
     */
    void
    start_receive_on_each_subnet
        ( void )
    {
        schedule_receive_on_subnet( ipv4_subnet_ );
        schedule_receive_on_subnet( ipv6_subnet_ );
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
                handle_new_message( sender, message );
                schedule_receive_on_subnet( current_subnet );
            }
        };

        current_subnet.async_receive( on_new_message );
    }

    /**
     *
     */
    void
    handle_new_message
        ( detail::message_socket::endpoint_type const& sender
        , detail::buffer const& message )
    {
        auto i = message.begin(), e = message.end();

        detail::header h;
        // Try to deserialize header.
        if ( deserialize( i, e, h ) )
            return;

        switch ( h.type_ )
        {
            case detail::header::PING_REQUEST: 
                handle_ping_request( sender, h );
                break;
            case detail::header::STORE_REQUEST: 
                handle_store_request( sender, h, i, e );
                break;
            case detail::header::FIND_NODE_REQUEST: 
                handle_find_node_request( sender, h, i, e );
                break;
            case detail::header::FIND_VALUE_REQUEST:
                handle_find_value_request( sender, h, i, e );
                break;
            default:
                handle_response( sender, h, i, e );
        }
    }

    /**
     *
     */
    void
    handle_response 
        ( detail::message_socket::endpoint_type const& sender
        , detail::header const& h
        , detail::buffer::const_iterator i
        , detail::buffer::const_iterator e )
    { response_dispatcher_.dispatch_message( sender, h, i, e ); }

    /**
     *
     */
    void
    add_current_peer_to_routing_table
        ( detail::id const& peer_id
        , detail::message_socket::endpoint_type const& peer_endpoint )
    { routing_table_.push( peer_id, peer_endpoint ); }

    /**
     *
     */
    void
    handle_ping_request
        ( detail::message_socket::endpoint_type const& sender
        , detail::header const& h )
    {
        add_current_peer_to_routing_table( h.source_id_, sender );

        // And respond to him.
        auto response = serialize_message( detail::header::PING_RESPONSE
                                         , h.random_token_ );
        async_send_response( response, sender ); 
    }

    /**
     *
     */
    void
    handle_store_request
        ( detail::message_socket::endpoint_type const& sender
        , detail::header const& h
        , detail::buffer::const_iterator i
        , detail::buffer::const_iterator e )
    {
        detail::store_value_request_body request;
        if ( detail::deserialize( i, e, request ) )
            return;

        add_current_peer_to_routing_table( h.source_id_, sender );

        value_store_[ request.data_key_hash_ ] 
                = std::move( request.data_value_ );
    }

    /**
     *
     */
    void
    send_find_node_response
        ( detail::message_socket::endpoint_type const& sender
        , detail::id const& random_token
        , detail::id const& node_to_find_id )
    {
        // Find X closest peers and save
        // their location into the response..
        detail::find_node_response_body response;

        auto remaining_node = ROUTING_TABLE_BUCKET_SIZE;
        for ( auto i = routing_table_.find( node_to_find_id )
                 , e = routing_table_.end()
            ; i != e && remaining_node > 0
            ; ++i, -- remaining_node )
            response.nodes_.push_back( { i->first, i->second } );

        // Now send the response.
        async_send_response( serialize_message( response, random_token )
                           , sender );
    }

    /**
     *
     */
    void
    handle_find_node_request
        ( detail::message_socket::endpoint_type const& sender
        , detail::header const& h
        , detail::buffer::const_iterator i
        , detail::buffer::const_iterator e )
    {
        // Ensure the request is valid.
        detail::find_node_request_body request;
        if ( detail::deserialize( i, e, request ) )
            return;

        add_current_peer_to_routing_table( h.source_id_, sender );

        send_find_node_response( sender
                               , h.random_token_
                               , request.node_to_find_id_ );
    }

    /**
     *
     */
    void
    handle_find_value_request
        ( detail::message_socket::endpoint_type const& sender
        , detail::header const& h
        , detail::buffer::const_iterator i
        , detail::buffer::const_iterator e )
    {
        detail::find_value_request_body request;
        if ( detail::deserialize( i, e, request ) )
            return;

        add_current_peer_to_routing_table( h.source_id_, sender );

        auto found = value_store_.find( request.value_to_find_ ); 
        if ( found == value_store_.end() )
            send_find_node_response( sender
                                   , h.random_token_
                                   , request.value_to_find_ );
        else
        {
            detail::find_value_response_body const response{ found->second };
            async_send_response( serialize_message( response, h.random_token_ )
                               , sender );
        }
    }

    /**
     *
     */
    detail::subnet &
    get_subnet_for
        ( detail::message_socket::endpoint_type const& e )
    {
        if ( e.address().is_v4() )
            return ipv4_subnet_;

        return ipv6_subnet_;
    }

    /**
     *
     */
    detail::header
    generate_header
        ( detail::header::type const& type
        , detail::id const& random_id )
    {
        return detail::header
                { detail::header::V1
                , type
                , my_id_
                , detail::id{ random_engine_ } };
    }

    /**
     *
     */
    template< typename Message >
    std::shared_ptr< detail::buffer >
    serialize_message
        ( Message const& message
        , detail::id const& response_id )
    {
        auto buffer = std::make_shared< detail::buffer >();

        auto const type = detail::message_traits< Message >::TYPE_ID;
        auto const header = generate_header( type, response_id );

        detail::serialize( header, *buffer );
        detail::serialize( message, *buffer );

        return std::move( buffer );
    }

    /**
     *
     */
    std::shared_ptr< detail::buffer >
    serialize_message
        ( detail::header::type const& type
        , detail::id const& response_id )
    {
        auto buffer = std::make_shared< detail::buffer >();

        auto const header = generate_header( type, response_id );

        detail::serialize( header, *buffer );

        return std::move( buffer );
    }

    /**
     *
     */
    template< typename OnResponseReceived, typename OnError >
    void
    register_temporary_association
        ( detail::id const& response_id
        , detail::timer::duration const& association_ttl
        , OnResponseReceived const& on_response_received
        , OnError const& on_error )
    {
        auto on_timeout = [ this, on_error, response_id ]
            ( void ) 
        {
            // If an association has been removed, that means
            // the message has never been received
            // hence report the timeout to the client.
            if ( response_dispatcher_.remove_association( response_id ) )
                on_error( make_error_code( std::errc::timed_out ) );
        };

        // Associate the response id with the 
        // on_response_received callback.
        response_dispatcher_.push_association( response_id
                                             , on_response_received );

        timer_.expires_from_now( association_ttl, on_timeout );
    }

    /**
     *
     */
    template< typename Request, typename OnResponseReceived, typename OnError >
    void
    async_send_request
        ( detail::id const& response_id
        , Request const& request
        , detail::message_socket::endpoint_type const& e
        , detail::timer::duration const& timeout
        , OnResponseReceived const& on_response_received
        , OnError const& on_error )
    { 
        // Generate the request buffer.
        auto message = serialize_message( request, response_id );

        // This lamba will keep the request message alive.
        auto on_request_sent = [ this, response_id
                               , on_response_received, on_error
                               , timeout, message ] 
            ( std::error_code const& failure ) 
        {
            if ( failure )
                on_error( failure );
            else 
                register_temporary_association( response_id, timeout
                                              , on_response_received
                                              , on_error );
        };

        // Serialize the request and send it.
        get_subnet_for( e ).async_send( *message, e, on_request_sent );
    }

    /**
     *
     */
    void
    async_send_response
        ( std::shared_ptr< detail::buffer > const& response
        , detail::message_socket::endpoint_type const& e )
    { 
        // This lamba will keep the response message alive.
        auto on_response_sent = [ response ] 
            ( std::error_code const& failure ) 
        { };

        // Serialize the message and send it.
        get_subnet_for( e ).async_send( *response, e, on_response_sent );
    }


    /**
     *
     */
    void
    discover_neighbors
        ( void )
    {
        // Initial peer should know our neighbors, hence ask 
        // him which peers are close to our own id.
        auto endoints_to_query = std::make_shared< detail::resolved_endpoints >
                ( detail::resolve_endpoint( io_service_, initial_peer_ ) );

        search_ourselves( std::move( endoints_to_query ) );
    }

    /**
     *
     */
    void
    search_ourselves
        ( std::shared_ptr< detail::resolved_endpoints > endpoints_to_query )
    { 
        if ( endpoints_to_query->empty() )
        {
            main_failure_ = make_error_code( INITIAL_PEER_FAILED_TO_RESPOND );
            return;
        }

        // Retrieve the next endpoint to query.
        auto const endpoint_to_query = endpoints_to_query->back();
        endpoints_to_query->pop_back();

        // On message received, process it.
        auto on_message_received = [ this ]
            ( detail::message_socket::endpoint_type const& s
            , detail::header const& h
            , detail::buffer::const_iterator i
            , detail::buffer::const_iterator e )
        { handle_initial_contact_response( s, h, i, e ); };

        // On error, retry with another endpoint.
        auto on_error = [ this, endpoints_to_query ]
            ( std::error_code const& ) 
        { search_ourselves( endpoints_to_query ); };

        async_send_request( detail::id{ random_engine_ }
                          , detail::find_node_request_body{ my_id_ }
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
        ( detail::message_socket::endpoint_type const& s
        , detail::header const& h
        , detail::buffer::const_iterator i
        , detail::buffer::const_iterator e )
    { 
        if ( h.type_ != detail::header::FIND_NODE_RESPONSE )
            return ;

        detail::find_node_response_body response;
        if ( detail::deserialize( i, e, response ) )
            return;

        // Add the initial peer to the routing_table.
        routing_table_.push( h.source_id_, s );

        // And its known peers.
        for ( auto const& node : response.nodes_ )
            routing_table_.push( node.id_, node.endpoint_ );
    }

    /**
     *
     */
    void
    send_store_request
        ( detail::id const& hashed_key
        , data_type const& data
        , save_handler_type handler )
    { 
        
    }

    /**
     *
     */
    void
    find_value
        ( std::shared_ptr< find_value_context > context )
    {
        auto candidates = context->select_n_candidates( CONCURRENT_FIND_NODE_REQUESTS_COUNT );

        // On message received, process it.
        auto on_message_received = [ this, context ]
            ( detail::message_socket::endpoint_type const& s
            , detail::header const& h
            , detail::buffer::const_iterator i
            , detail::buffer::const_iterator e )
        { 
            context->ack_candidate_response();
            handle_find_node_response( s, h, i, e, context );
        };

        // On error, retry with another endpoint.
        auto on_error = [ this, context ]
            ( std::error_code const& ) 
        {
            // XXX: Current candidate must be flagged as stale.
            context->ack_candidate_response();
            find_value( context ); 
        };

        detail::find_node_request_body request{ context->get_searched_key() };

        for ( auto const& candidate : candidates )
            async_send_request( detail::id{ random_engine_ }
                              , request
                              , candidate
                              , NODE_LOOKUP_TIMEOUT 
                              , on_message_received
                              , on_error );
    }

    /**
     *
     */
    std::shared_ptr< find_value_context >
    create_find_value_context 
        ( detail::id const& key
        , load_handler_type load_handler )
    {
        auto i = routing_table_.find( key ), e = routing_table_.end();
        return std::make_shared< find_value_context >( key, i, e
                                                     , load_handler );
    }

    /**
     *
     */
    void
    handle_find_node_response
        ( detail::message_socket::endpoint_type const& s
        , detail::header const& h
        , detail::buffer::const_iterator i
        , detail::buffer::const_iterator e
        , std::shared_ptr< find_value_context > context )
    { 
        // Add the initial peer to the routing_table.
        add_current_peer_to_routing_table( h.source_id_, s );

        if ( h.type_ == detail::header::FIND_NODE_RESPONSE )
            handle_find_node_response( i, e, context );
        else if ( h.type_ == detail::header::FIND_VALUE_RESPONSE )
            handle_find_value_response( i, e, context );
    }

    /**
     *
     */
    void
    handle_find_node_response
        ( detail::buffer::const_iterator i
        , detail::buffer::const_iterator e
        , std::shared_ptr< find_value_context > context )
    { 
        detail::find_node_response_body response;
        if ( detail::deserialize( i, e, response ) )
            return;

        // Keep track of the closest candidate before
        // new candidate insertion.
        auto closest_candidate = context->get_closest_candidate();
        for ( auto const& new_peer : response.nodes_ )
            context->add_candidate( new_peer.id_, new_peer.endpoint_ );

        // If a closer candidate has been found, keep going
        // asking for closer candidate.
        if ( closest_candidate != context->get_closest_candidate() )
            find_value( context );
        else if ( context->have_all_requests_completed() )
            context->forward_to_client( make_error_code( VALUE_NOT_FOUND ) );
    }

    /**
     *
     */
    void
    handle_find_value_response
        ( detail::buffer::const_iterator i
        , detail::buffer::const_iterator e
        , std::shared_ptr< find_value_context > context )
    { 
        detail::find_value_response_body response;
        if ( detail::deserialize( i, e, response ) )
            return;

        context->forward_to_client( response.data_ );
    }


private:
    std::default_random_engine random_engine_;
    detail::id my_id_;
    boost::asio::io_service io_service_;
    endpoint initial_peer_;
    detail::subnet ipv4_subnet_;
    detail::subnet ipv6_subnet_;
    detail::routing_table routing_table_;
    detail::response_dispatcher response_dispatcher_;
    detail::timer timer_;
    detail::value_store< detail::id, data_type > value_store_;
    std::error_code main_failure_;
};

session::session
    ( endpoint const& initial_peer
    , endpoint const& listen_on_ipv4
    , endpoint const& listen_on_ipv6 )
    : impl_{ new impl{ initial_peer, listen_on_ipv4, listen_on_ipv6 } }
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
