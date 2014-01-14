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

#ifndef KADEMLIA_PEER_HPP
#define KADEMLIA_PEER_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <iosfwd>
#include <string>
#include <vector>
#include <cassert>

#include <kademlia/endpoint.hpp>
#include <kademlia/detail/cxx11_macros.hpp>

namespace kademlia {
namespace detail {

/**
 *
 */
class peer final
{
public:
    using endpoints_type = std::vector< endpoint >;

public:
    /**
     *
     */
    explicit 
    peer
        ( endpoints_type const& endpoints );

    /**
     *
     */
    bool
    operator==
        ( peer const& o )
        const;

    /**
     *
     */
    bool
    operator!=
        ( peer const& o )
        const;

    /**
     *
     */
    endpoints_type const&
    get_endpoints
        ( void )
        const;

private:
    ///
    endpoints_type endpoints_;
};

/**
 *
 */
std::ostream&
operator<<
    ( std::ostream & out
    , peer const& p );

inline
peer::peer
    ( endpoints_type const& endpoints )
    : endpoints_( endpoints )
{ assert( ! endpoints_.empty() && "peer's endpoints list must not be empty" ); }

inline bool
peer::operator==
    ( peer const& o )
    const
{ return endpoints_ == o.endpoints_; }

inline bool
peer::operator!=
    ( peer const& o )
    const
{ return ! this->operator==( o ); }

inline peer::endpoints_type const&
peer::get_endpoints
    ( void )
    const
{ return endpoints_; }


} // namespace detail
} // namespace kademlia

#endif

