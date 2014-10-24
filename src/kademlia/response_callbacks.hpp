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

#ifndef KADEMLIA_RESPONSE_CALLBACKS_HPP
#define KADEMLIA_RESPONSE_CALLBACKS_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <map>
#include <functional>

#include "kademlia/id.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/message.hpp"

namespace kademlia {
namespace detail {

///
class response_callbacks final
{
public:
    ///
    using endpoint_type = ip_endpoint;

    ///
    using callback = std::function< void
            ( endpoint_type const& sender
            , header const& h
            , buffer::const_iterator i
            , buffer::const_iterator e ) >;

public:
    /**
     *
     */
    void
    push_callback
        ( id const& message_id
        , callback const& on_message_received );

    /**
     *
     */
    bool
    remove_callback
        ( id const& message_id );

    /**
     *
     */
    std::error_code
    dispatch_response
        ( endpoint_type const& sender
        , header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e );

private:
    ///
    using callbacks = std::map< id, callback >;

private:
    ///
    callbacks callbacks_;
};

} // namespace detail
} // namespace kademlia

#endif

