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

/**
 *  @brief This object represents peers and listening addresses.
 */
class endpoint final
{
public:
    /// The IP/hostname address.
    using address_type = std::string;
    /// The port identifier.
    using service_type = std::string;
    /// The port number.
    using service_numeric_type = std::uint16_t;

    /**
     *  @brief Construct an endpoint from an address and a port identifier.
     *
     *  @param address The ip/hostname (e.g. www.example.com, 127.0.0.1).
     *  @param service The port identifier (e.g. http, 80)
     */
    endpoint
        ( address_type const& address
        , service_type const& service )
            : address_( address )
            , service_( service )
    { }

    /**
     *  @brief Construct an endpoint from an address and a port number.
     *
     *  @param address The ip/hostname (e.g. www.example.com, 127.0.0.1).
     *  @param service The port number (e.g.  80)
     */
    endpoint
        ( address_type const& address
        , service_numeric_type const& service )
            : address_( address )
            , service_( std::to_string( service ) )
    { }

    /**
     *  @brief Get the endpoint address.
     *
     *  @return The IP/hostname.
     */
    address_type const&
    address
        ( void )
        const
    { return address_; }

    /**
     *  @brief Set the endpoint address.
     *
     *  @param address The IP/hostname.
     */
    void
    address
        ( address_type const& address )
    { address_ = address; }

    /**
     *  @brief Get the endpoint port.
     *
     *  @return The port number/identifier.
     */
    service_type const&
    service
        ( void )
        const
    { return service_; }

    /**
     *  @brief Set the endpoint port.
     *
     *  @param service The port number/identifier.
     */
    void
    service
        ( service_type const& service )
    { service_ = service; }

private:
    ///
    address_type address_;
    ///
    service_type service_;
};

KADEMLIA_SYMBOL_VISIBILITY
std::ostream&
operator<<
    ( std::ostream & out
    , endpoint const& e );

/**
 *  @brief Compare two endpoints for equality.
 *
 *  @param a The first endpoint to compare.
 *  @param b The second endpoint to compare.
 *  @return true if a & b are equals, false otherwise.
 */
inline bool
operator==
    ( endpoint const& a
    , endpoint const& b )
{ return a.address() == b.address() && a.service() == b.service(); }

/**
 *  @brief Compare two endpoints for inequality.
 *
 *  @param a The first endpoint to compare.
 *  @param b The second endpoint to compare.
 *  @return false if a & b are equals, true otherwise.
 */
inline bool
operator!=
    ( endpoint const& a
    , endpoint const& b )
{ return ! ( a == b ); }

} // namespace kademlia

#endif

