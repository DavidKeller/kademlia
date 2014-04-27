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

#ifndef KADEMLIA_SESSION_HPP
#define KADEMLIA_SESSION_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <system_error>

#include <kademlia/detail/symbol_visibility.hpp>
#include <kademlia/detail/cxx11_macros.hpp>
#include <kademlia/endpoint.hpp>

namespace kademlia {

/**
 *
 */
class session final
{
public:
    ///
    using key_type = std::vector< std::uint8_t >;
 
    ///
    using data_type = std::vector< std::uint8_t >;
 
    ///
    using save_handler_type = std::function 
            < void 
                ( std::error_code const& error )
            >;
    ///
    using load_handler_type = std::function 
            < void 
                ( std::error_code const& error
                , data_type const& data )
            >;

    ///
    enum { DEFAULT_PORT = 27980 };

public:
    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    session
        ( endpoint const& initial_peer
        , endpoint const& listen_on_ipv4 = endpoint{ "0.0.0.0", DEFAULT_PORT }
        , endpoint const& listen_on_ipv6 = endpoint{ "::", DEFAULT_PORT } );

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    ~session
        ( void );

    /**
     * Disabled copy constructor.
     */
    session
        ( session const& )
        = delete;

    /**
     * Disabled assignement operator.
     */
    session&
    operator=
        ( session const& )
        = delete;

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    void
    async_save
        ( key_type const& key 
        , data_type const& data
        , save_handler_type handler );

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    void
    async_load
        ( key_type const& key
        , load_handler_type handler );

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    std::error_code
    run
        ( void );

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    void
    abort
        ( void );

private:
    //
    class impl;

private:
    //
    std::unique_ptr< impl > impl_;
};

} // namespace kademlia

#endif

