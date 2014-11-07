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

#ifndef KADEMLIA_FIRST_SESSION_HPP
#define KADEMLIA_FIRST_SESSION_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <memory>
#include <system_error>

#include <kademlia/detail/symbol_visibility.hpp>
#include <kademlia/detail/cxx11_macros.hpp>
#include <kademlia/endpoint.hpp>
#include <kademlia/session_base.hpp>

namespace kademlia {

/**
 *  @brief This object is used to bootstrap a network.
 */
class first_session final
        : public session_base
{
public:
    /**
     *  @brief Construct a passive first_session.
     *  @details This first_session acts like an active first_session except it
     *           does'nt try to discover neighbors. It can be used
     *           by the first node of a network as no peer is known
     *           uppon its creation.
     *
     *           It does'nt make sense to use this constructor once the
     *           network has at least one peer.
     *
     *  @param listen_on_ipv4 IPv4 listening endpoint.
     *  @param listen_on_ipv6 IPv6 listening endpoint.
     */
    KADEMLIA_SYMBOL_VISIBILITY
    first_session
        ( endpoint const& listen_on_ipv4 = endpoint{ "0.0.0.0", DEFAULT_PORT }
        , endpoint const& listen_on_ipv6 = endpoint{ "::", DEFAULT_PORT } );

    /**
     *  @brief Destruct the first_session.
     */
    KADEMLIA_SYMBOL_VISIBILITY
    ~first_session
        ( void );

    /**
     *  @brief Disabled copy constructor.
     */
    first_session
        ( first_session const& )
        = delete;

    /**
     *  @brief Disabled assignment operator.
     */
    first_session&
    operator=
        ( first_session const& )
        = delete;

    /**
     *  @brief This <b>blocking call</b> execute the first_session main loop.
     *
     *  @return The exit reason of the call.
     */
    KADEMLIA_SYMBOL_VISIBILITY
    std::error_code
    run
        ( void );

    /**
     *  @brief Abort the first_session main loop.
     */
    KADEMLIA_SYMBOL_VISIBILITY
    void
    abort
        ( void );

private:
    /// Hidden implementation.
    struct impl;

private:
    /// The hidden implementation instance.
    std::unique_ptr< impl > impl_;
};

} // namespace kademlia

#endif

