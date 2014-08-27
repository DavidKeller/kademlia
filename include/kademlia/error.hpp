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

namespace kademlia {

/**
 *
 */
enum error_type
{
    UNKNOWN = 1,
    RUN_ABORTED,
    INITIAL_PEER_FAILED_TO_RESPOND,
    INVALID_ID,
    TRUNCATED_ID,
    TRUNCATED_HEADER,
    TRUNCATED_ENDPOINT,
    TRUNCATED_ADDRESS,
    TRUNCATED_SIZE,
    UNKNOWN_PROTOCOL_VERSION,
    CORRUPTED_HEADER,
    CORRUPTED_BODY,
    UNASSOCIATED_MESSAGE_ID,
    INVALID_IPV4_ADDRESS,
    INVALID_IPV6_ADDRESS,
    UNIMPLEMENTED,
    NO_VALID_NEIGHBOR_REMAINING,
    VALUE_NOT_FOUND,
    TIMER_MALFUNCTION,
    ALREADY_RUNNING,
};

/**
 *
 */
std::error_category const&
error_category
    ( void );

/**
 *
 */
std::error_condition
make_error_condition
    ( error_type condition );

/**
 *
 */
std::error_code
make_error_code
    ( error_type code );

} // namespace kademlia

namespace std {

template <>
struct is_error_condition_enum<kademlia::error_type> : true_type {};

} // namespace std

#endif

