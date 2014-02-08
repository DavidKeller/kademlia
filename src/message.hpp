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

#include <cstdint>
#include <algorithm>
#include <system_error>

#include <kademlia/detail/cxx11_macros.hpp>

#include "message_socket.hpp"
#include "id.hpp"
#include "buffer.hpp"

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
        FIND_NODE_REQUEST,
        ///
        FIND_NODE_RESPONSE,
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
struct find_node_request_body final
{
    ///
    id node_to_find_id_;
};

/**
 *
 */
void
serialize
    ( find_node_request_body const& body
    , buffer & b );

/**
 *
 */
inline std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_node_request_body & body );

/**
 *
 */
struct node
{
    id id_;
    message_socket::endpoint_type endpoint_;
};

/**
 *
 */
struct find_node_response_body final
{
    ///
    std::vector<node> nodes_;
};

/**
 *
 */
void
serialize
    ( find_node_response_body const& body
    , buffer & b );

/**
 *
 */
std::error_code
deserialize
    ( buffer::const_iterator & i
    , buffer::const_iterator e
    , find_node_response_body & body );

} // namespace detail
} // namespace kademlia

#endif

