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

#ifndef KADEMLIA_ENDPOINT_HPP
#define KADEMLIA_ENDPOINT_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <string>
#include <cstdint>
#include <iosfwd>
#include <sstream>

#include <kademlia/detail/symbol_visibility.hpp>
#include <kademlia/detail/cxx11_macros.hpp>

namespace kademlia {

class endpoint final
{
public:
    using address_type = std::string;
    using service_type = std::string;
    using service_numeric_type = std::uint16_t;

public:
    endpoint
        ( void )
    { }

    endpoint
        ( address_type const& address
        , service_type const& service )
            : address_( address )
            , service_( service )
    { }

    endpoint
        ( address_type const& address
        , service_numeric_type const& service )
            : address_( address )
            , service_( std::to_string( service ) )
    { }

    address_type const&
    address
        ( void )
        const
    { return address_; }

    void
    address
        ( address_type const& address )
    { address_ = address; }

    service_type const&
    service
        ( void )
        const
    { return service_; }

    void
    service
        ( service_type const& service )
    { service_ = service; }

private:
    address_type address_;
    service_type service_;
};

KADEMLIA_SYMBOL_VISIBILITY
std::ostream&
operator<<
    ( std::ostream & out
    , endpoint const& e );

KADEMLIA_SYMBOL_VISIBILITY
std::istream &
operator>>
    ( std::istream & in
    , endpoint & e );

inline bool
operator==
    ( endpoint const& a
    , endpoint const& b )
{ return a.address() == b.address() && a.service() == b.service(); }

inline bool
operator!=
    ( endpoint const& a
    , endpoint const& b )
{ return ! ( a == b ); }

} // namespace kademlia

#endif

