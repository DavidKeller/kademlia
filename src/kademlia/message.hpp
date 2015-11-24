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

#ifndef KADEMLIA_MESSAGE_HPP
#define KADEMLIA_MESSAGE_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <iosfwd>
#include <cstdint>
#include <algorithm>
#include <system_error>
#include <vector>

#include <boost/asio/ip/address.hpp>

#include <kademlia/detail/cxx11_macros.hpp>

#include "kademlia/peer.hpp"
#include "kademlia/id.hpp"
#include "kademlia/buffer.hpp"

namespace kademlia {
namespace detail {

/**
 *
 */
struct header final
{
    enum version : std::uint8_t
    {
        ///
        V1 = 1,
    } version_;

    ///
    enum type : std::uint8_t
    {
        ///
        PING_REQUEST,
        ///
        PING_RESPONSE,
        ///
        STORE_REQUEST,
        ///
        FIND_PEER_REQUEST,
        ///
        FIND_PEER_RESPONSE,
        ///
        FIND_VALUE_REQUEST,
        ///
        FIND_VALUE_RESPONSE,
    } type_;

    ///
    id source_id_;
    ///
    id random_token_;
};

/**
 *
 */
std::ostream &
operator<<
    ( std::ostream & out
    , header::type const& h );

/**
 *
 */
std::ostream &
operator<<
    ( std::ostream & out
    , header const& h );

/**
 *
 */
template< typename MessageBodyType >
struct message_traits;

/**
 *
 */
void
serialize
    ( header const& h
    , buffer & b );

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , header & h );

/**
 *
 */
struct find_peer_request_body final
{
    ///
    id peer_to_find_id_;
};

/**
 *
 */
template<>
struct message_traits< find_peer_request_body >
{ static CXX11_CONSTEXPR header::type TYPE_ID = header::FIND_PEER_REQUEST; };

/**
 *
 */
void
serialize
    ( find_peer_request_body const& body
    , buffer & b );

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_peer_request_body & body );


/**
 *
 */
struct find_peer_response_body final
{
    ///
    std::vector< peer > peers_;
};

/**
 *
 */
template<>
struct message_traits< find_peer_response_body >
{ static CXX11_CONSTEXPR header::type TYPE_ID = header::FIND_PEER_RESPONSE; };

/**
 *
 */
void
serialize
    ( find_peer_response_body const& body
    , buffer & b );

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_peer_response_body & body );

/**
 *
 */
struct find_value_request_body final
{
    ///
    id value_to_find_;
};

/**
 *
 */
template<>
struct message_traits< find_value_request_body >
{ static CXX11_CONSTEXPR header::type TYPE_ID = header::FIND_VALUE_REQUEST; };

/**
 *
 */
void
serialize
    ( find_value_request_body const& body
    , buffer & b );

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_value_request_body & body );

/**
 *
 */
struct find_value_response_body final
{
    ///
    std::vector< std::uint8_t > data_;
};

/**
 *
 */
template<>
struct message_traits< find_value_response_body >
{ static CXX11_CONSTEXPR header::type TYPE_ID = header::FIND_VALUE_RESPONSE; };


/**
 *
 */
void
serialize
    ( find_value_response_body const& body
    , buffer & b );

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_value_response_body & body );

/**
 *
 */
struct store_value_request_body final
{
    ///
    id data_key_hash_;
    ///
    std::vector< std::uint8_t > data_value_;
};

/**
 *
 */
template<>
struct message_traits< store_value_request_body >
{ static CXX11_CONSTEXPR header::type TYPE_ID = header::STORE_REQUEST; };


/**
 *
 */
void
serialize
    ( store_value_request_body const& body
    , buffer & b );

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , store_value_request_body & body );

} // namespace detail
} // namespace kademlia

#endif

