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

#include "kademlia/id.hpp"

#include <cctype>
#include <iostream>
#include <iterator>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <utility>
#include <sstream>
#include <iomanip>

#include <openssl/sha.h>

#include "kademlia/error_impl.hpp"

namespace kademlia {
namespace detail {

static CXX11_CONSTEXPR std::size_t HEX_CHAR_PER_BLOCK = id::BYTE_PER_BLOCK * 2;

namespace {

id::block_type
to_block
    ( std::string const& s )
{
    auto is_id_digit = []( int c ) { return std::isxdigit( c ); };
    if ( ! std::all_of( s.begin(), s.end(), is_id_digit ) )
        throw std::system_error{ make_error_code( INVALID_ID ) };

    std::stringstream converter{ s };

    std::uint64_t result;
    converter >> std::hex >> result;

    assert( ! converter.fail() && "hexa to decimal conversion failed" );

    return static_cast< id::block_type >( result );
}

} // namespace

id::id
    ( std::default_random_engine & random_engine )
{
    // The output of the generator is treated as boolean value.
    std::uniform_int_distribution<> distribution
            ( std::numeric_limits< block_type >::min()
            , std::numeric_limits< block_type >::max() );

    std::generate( blocks_.begin(), blocks_.end()
                 , std::bind( distribution, std::ref( random_engine ) ) );
}

id::id
    ( std::string s )
{
    auto CXX11_CONSTEXPR STRING_MAX_SIZE = BLOCKS_COUNT * HEX_CHAR_PER_BLOCK;

    if ( s.size() > STRING_MAX_SIZE )
        throw std::system_error{ make_error_code( INVALID_ID ) };

    // Insert leading 0.
    s.insert( s.begin(), STRING_MAX_SIZE - s.size(), '0' );

    assert( s.size() == STRING_MAX_SIZE && "string padding failed" );
    for ( std::size_t i = 0; i != BLOCKS_COUNT; ++ i )
        blocks_[ i ] = to_block( s.substr( i * HEX_CHAR_PER_BLOCK
                                         , HEX_CHAR_PER_BLOCK ) );
}

id::id
    ( value_to_hash_type const& value )
{
    // Use OpenSSL crypto hash.
    SHA1( value.data(), value.size(), blocks_.data() );
}

std::ostream &
operator<<
    ( std::ostream & out
    , id const& id_to_print )
{
    auto e = id_to_print.end();

    // Skip leading 0.
    auto is_not_0 = []( id::block_type b ) { return b != 0; };
    auto i = std::find_if( id_to_print.begin(), e, is_not_0 );

    auto const previous_flags = out.flags();

    out << std::hex
        << std::setfill( '0' )
        << std::setw( HEX_CHAR_PER_BLOCK );

    std::copy( i, e, std::ostream_iterator< std::uint64_t >{ out } );

    out.flags( previous_flags );

    return out;
}

} // namespace detail
} // namespace kademlia
