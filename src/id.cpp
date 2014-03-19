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

#include "id.hpp"

#include <iostream>
#include <iterator>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <utility>
#include <sstream>
#include <iomanip>

#include <openssl/sha.h>

#include <kademlia/error.hpp>

namespace kademlia {
namespace detail {

static CXX11_CONSTEXPR std::size_t HEX_CHAR_PER_BLOCK = id::BYTE_PER_BLOCK * 2;

namespace {

id::block_type
to_block
    ( std::string const& value )
{
    std::stringstream converter{ value };

    std::uint64_t result;
    converter >> std::hex >> result;

    if ( converter.fail() )
        throw std::system_error{ make_error_code( INVALID_ID ) };

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
    
    for ( std::size_t i = 0; i < BLOCKS_COUNT; ++ i )
        blocks_[i] = distribution( random_engine );
}

id::id
    ( std::string const& s )
    : blocks_{ }
{
    if ( s.size() > BLOCKS_COUNT * HEX_CHAR_PER_BLOCK )
        throw std::system_error{ make_error_code( INVALID_ID ) };

    // Parse string from MSB to LSB.
    std::size_t i = 0, e = s.size();

    // Check if the first block contains less than HEX_CHAR_PER_BLOCK.
    std::size_t const first_block_char_count = e % HEX_CHAR_PER_BLOCK; 
    if ( first_block_char_count != 0 )
    {
        auto const value = to_block( s.substr( i, first_block_char_count ) );
        i += first_block_char_count;
        auto remaining_char_count = e - i;
        blocks_[ remaining_char_count / HEX_CHAR_PER_BLOCK ] = value;
    }

    // Parse any remaining complete block.
    while ( i != e )
    {
        auto const value = to_block( s.substr( i, HEX_CHAR_PER_BLOCK ) );
        i += HEX_CHAR_PER_BLOCK;
        auto remaining_char_count = e - i;
        blocks_[ remaining_char_count / HEX_CHAR_PER_BLOCK ] = value;
    }
}
    
id::id 
    ( value_to_hash_type const& value )
{
    static_assert( BIT_SIZE == SHA_DIGEST_LENGTH * 8
                 , "An id can't be constructed from a sha1 result" );
    // Use openssl SHA1 here.
    ::SHA1( value.data(), value.size(), blocks_.data() );
}

std::ostream &
operator<<
    ( std::ostream & out
    , id const& id_to_print )
{
    auto i = id_to_print.begin_block(), e = id_to_print.end_block();

    // Skip leading 0.
    while ( i != e && *i == 0 )
        ++ i;

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
