// Copyright (c) 2010, David Keller
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

#if defined(_MSC_VER)
#   pragma once
#endif

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <system_error>
#ifndef HAS_CXX11_METHOD_SPECIFIER
#   include <boost/noncopyable.hpp>
#endif

#include <kademlia/detail/symbol_visibility.hpp>
#include <kademlia/detail/cxx11_macros.hpp>
#include <kademlia/endpoint.hpp>

namespace kademlia {

/**
 *
 */
class session
#ifdef HAS_CXX11_FINAL
    final
#endif
#ifndef HAS_CXX11_METHOD_SPECIFIER
    : private boost::noncopyable
#endif
{
public:
    //
    typedef std::function 
            < void 
                ( std::error_code const& error )
            > save_handler_type;
    //
    typedef std::function 
            < void 
                ( std::error_code const& error
                , std::string const& data )
            > load_handler_type;
 
public:
    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    explicit
    session
        ( std::vector< endpoint > const& listening_endpoints
        , endpoint const& initial_peer );

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    ~session
        ( void );

#ifdef HAS_CXX11_METHOD_SPECIFIER
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
#endif

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    void
    async_save
        ( std::string const& key 
        , std::string const& data
        , save_handler_type handler );

    /**
     *
     */
    KADEMLIA_SYMBOL_VISIBILITY
    void
    async_load
        ( std::string const& key
        , load_handler_type handler );

private:
    //
    class impl;

private:
    //
    std::unique_ptr< impl > impl_;
};

}

#endif

