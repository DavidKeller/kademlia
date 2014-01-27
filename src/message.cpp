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

#include "message.hpp"

#include <kademlia/error.hpp>

namespace kademlia {
namespace detail {

namespace {

inline void
serialize
    ( id const& i
    , buffer & b )
{
    std::copy( i.begin_block()
             , i.end_block()
             , std::back_inserter( b ) );
}

/**
 *
 */
inline std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , id & new_id )
{
    if ( std::distance( i, e ) < id::BLOCKS_COUNT )
        return make_error_code( TRUNCATED_ID );

    std::copy_n( i, id::BLOCKS_COUNT, new_id.begin_block() );
    std::advance( i, id::BLOCKS_COUNT );

    return std::error_code{};
}

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , header::version & v
    , header::type & t )
{
    if ( std::distance( i, e ) < 1 )
        return make_error_code( TRUNCATED_HEADER ); 

    v = static_cast< header::version >( *i & 0xf );
    t = static_cast< header::type >( *i >> 4 );

    if ( v != header::V1 )
        return make_error_code( UNKNOWN_PROTOCOL_VERSION );

    std::advance( i, 1 );

    return std::error_code{};
}

enum 
    { KADEMLIA_ENDPOINT_SERIALIZATION_IPV4 = 1
    , KADEMLIA_ENDPOINT_SERIALIZATION_IPV6 = 2 };

/**
 *
 */
inline void
serialize
    ( message_socket::endpoint_type const& endpoint
    , buffer & b )
{
    auto const port = endpoint.port();
    b.push_back( port & 0xff );
    b.push_back( port >> 8 );

    if ( endpoint.address().is_v4() )
    {
        b.push_back( KADEMLIA_ENDPOINT_SERIALIZATION_IPV4 );
        auto const& a = endpoint.address().to_v4().to_bytes();
        b.insert( b.end(), a.begin(), a.end() );
    }
    else
    {
        assert( endpoint.address().is_v6() );
        b.push_back( KADEMLIA_ENDPOINT_SERIALIZATION_IPV6 );
        auto const& a = endpoint.address().to_v6().to_bytes();
        b.insert( b.end(), a.begin(), a.end() );
    }
}

/**
 *
 */
template<typename Address>
inline std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , Address & address )
{
    typename Address::bytes_type buffer;
    if ( std::distance( i, e ) < buffer.size() )
        return make_error_code( TRUNCATED_ADDRESS );
    
    std::copy_n( i, buffer.size(), buffer.begin() );
    std::advance( i, buffer.size() );

    address = Address{ buffer };

    return std::error_code{};
}

/**
 *
 */
inline std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , message_socket::endpoint_type & endpoint )
{
    if ( std::distance( i, e ) < 3 )
        return make_error_code( TRUNCATED_ENDPOINT ); 

    std::uint16_t port;
    port = *i++;
    port |= *i++ << 8;
    endpoint.port( port );

    auto const protocol = *i++;
    if ( protocol == KADEMLIA_ENDPOINT_SERIALIZATION_IPV4 )
    {
        boost::asio::ip::address_v4 address;
        auto const failure = deserialize( i, e, address );
        if ( failure )
            return failure;

        endpoint.address( address );
    }
    else
    {
        assert( protocol == KADEMLIA_ENDPOINT_SERIALIZATION_IPV6 );

        boost::asio::ip::address_v6 address;
        auto const failure = deserialize( i, e, address );
        if ( failure )
            return failure;

        endpoint.address( address );
    }

    return std::error_code{};
}

/**
 *
 */
inline void
serialize
    ( node const& n  
    , buffer & b )
{
    serialize( n.id_, b ); 
    serialize( n.endpoint_, b ); 
}

/**
 *
 */
inline std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , node & n )
{
    auto failure = deserialize( i, e, n.id_ ); 
    if ( failure )
        return failure;

    return deserialize( i, e, n.endpoint_ ); 
}

} // anonymous namespace

void
serialize
    ( header const& h
    , buffer & b )
{
    b.push_back( h.version_ | h.type_ << 4 );
    serialize( h.source_id_, b );
    serialize( h.random_token_, b );
}

std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , header & h )
{
    auto failure = deserialize( i, e, h.version_, h.type_ );
    if ( failure )
        return failure;

    failure = deserialize( i, e, h.source_id_ );
    if ( failure )
        return failure;

    return deserialize( i, e, h.random_token_ );
}

void
serialize
    ( find_node_request_body const& body
    , buffer & b )
{
    serialize( body.node_to_find_id_, b );
}

std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_node_request_body & body )
{
    return deserialize( i, e, body.node_to_find_id_ );
}

void
serialize
    ( find_node_response_body const& body
    , buffer & b )
{
    std::uint16_t const size = body.nodes_.size();
    b.push_back( size & 0xf );
    b.push_back( size >> 8 );

    for ( auto const & n : body.nodes_ )
        serialize( n, b );
}

std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_node_response_body & body )
{
    if ( std::distance( i, e ) < 2 )
        return make_error_code( TRUNCATED_ENDPOINT ); 

    std::uint16_t size;
    size = *i++;
    size |= *i++ << 8;
   
    std::error_code failure;

    for ( 
        ; size > 0 && ! failure
        ; -- size )
    {
        body.nodes_.resize( body.nodes_.size() + 1 );
        failure = deserialize( i, e, body.nodes_.back() );
    }

    return failure;
}

} // namespace detail
} // namespace kademlia
