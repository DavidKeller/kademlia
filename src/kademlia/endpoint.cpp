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

#include <kademlia/endpoint.hpp>

#include <cassert>

#include <ostream>
#include <istream>
#include <cctype>

namespace kademlia {

namespace {

std::string
parse_ipv6( std::istream & in )
{
    std::string ipv6;

    char c = in.get();
    assert( c == '[' && "An IPv6 address starts with '['");

    for ( c = in.get(); std::isalnum( c ) || c == ':'; c = in.get() )
        ipv6.push_back(c);

    if ( ipv6.empty() || c != ']' )
        in.setstate( std::istream::failbit );

    return ipv6;
}

std::string
parse_ipv4( std::istream & in )
{
    std::string ipv4;

    while ( std::isdigit( in.peek() ) || in.peek() == '.' )
        ipv4.push_back( in.get() );

    if ( ipv4.empty() )
        in.setstate( std::istream::failbit );

    return ipv4;
}

std::string
parse_ip( std::istream & in )
{
    // Check if the address is an IPv6
    if ( in.peek() == '[' )
        return parse_ipv6( in );

    return parse_ipv4( in );
}

std::string
parse_service( std::istream & in )
{
    std::string service;

    in >> service;

    return service;
}

}

std::istream&
operator>>
    ( std::istream & in
    , endpoint & e )
{
    std::istream::sentry s{ in };

    if ( s )
    {
        std::string const address = parse_ip( in );

        if ( in.get() != ':' )
            in.setstate( std::istream::failbit );

        std::string const service = parse_service( in );

        if ( in )
            e = endpoint{ address, service };
    }

    return in;
}

std::ostream&
operator<<
    ( std::ostream & out
    , endpoint const& e )
{
    std::ostream::sentry s{ out };

    if ( s )
        out << e.address() << ":" << e.service();

    return out;
}

} // namespace kademlia
