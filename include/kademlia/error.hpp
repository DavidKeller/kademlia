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

#ifndef KADEMLIA_ERROR_HPP
#define KADEMLIA_ERROR_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <system_error>
#include <kademlia/detail/symbol_visibility.hpp>

namespace kademlia {

/// This enum list all library specific errors.
enum error_type
{
    /// An unknown error.
    UNKNOWN_ERROR = 1,
    /// The session::abort() has been called.
    RUN_ABORTED,
    /// The session failed to contact a valid peer uppon creation.
    INITIAL_PEER_FAILED_TO_RESPOND,
    /// The session routing table is missing peers.
    MISSING_PEERS,
    /// An id has been corrupted.
    INVALID_ID,
    /// An id has been truncated.
    TRUNCATED_ID,
    /// A packet header from the network is corrupted.
    TRUNCATED_HEADER,
    /// An endpoint information has been corrupted.
    TRUNCATED_ENDPOINT,
    /// An endpoint address has been corrupted.
    TRUNCATED_ADDRESS,
    /// A list has been corrupted.
    TRUNCATED_SIZE,
    /// A message from an unknown version of the library has been received.
    UNKNOWN_PROTOCOL_VERSION,
    /// A packet body has been corrupted.
    CORRUPTED_BODY,
    /// An unexpected response has been received.
    UNASSOCIATED_MESSAGE_ID,
    /// The provided IPv4 address is invalid.
    INVALID_IPV4_ADDRESS,
    /// The provided IPv6 address is invalid.
    INVALID_IPV6_ADDRESS,
    /// The function/method has been implemented yet.
    UNIMPLEMENTED,
    /// The value associated with the requested key has not been found.
    VALUE_NOT_FOUND,
    /// The internal timer failed to tick.
    TIMER_MALFUNCTION,
    /// Another call to session::run() is still blocked.
    ALREADY_RUNNING,
};

/**
 *  @brief Create a library error condition.
 *
 *  @return The created error condition.
 */
KADEMLIA_SYMBOL_VISIBILITY std::error_condition
make_error_condition
    ( error_type condition );

} // namespace kademlia

namespace std {

template <>
struct is_error_condition_enum< kademlia::error_type > : true_type {};

} // namespace std

#endif

