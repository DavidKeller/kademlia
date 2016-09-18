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

#ifndef KADEMLIA_ID_HPP
#define KADEMLIA_ID_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <cstdint>
#include <array>
#include <iosfwd>
#include <random>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

#include <kademlia/detail/cxx11_macros.hpp>

namespace kademlia {
namespace detail {

///
class id final
{
public:
    ///
    static CXX11_CONSTEXPR std::size_t BIT_SIZE = 160;

    ///
    using block_type = std::uint8_t;

    ///
    static CXX11_CONSTEXPR std::size_t BYTE_PER_BLOCK = sizeof( block_type );

    ///
    static CXX11_CONSTEXPR std::size_t BIT_PER_BLOCK = BYTE_PER_BLOCK * 8;

    ///
    static CXX11_CONSTEXPR std::size_t BLOCKS_COUNT = BIT_SIZE / BIT_PER_BLOCK;

    ///
    using blocks_type = std::array< block_type, BLOCKS_COUNT >;

    ///
    using value_to_hash_type = std::vector< std::uint8_t >;

    /**
     *
     */
    template< typename BlockType >
    struct abstract_reference
    {
        /**
         *
         */
        explicit
        operator const bool
            ( void )
            const
        { return ( current_block_ & mask_ ) != 0; }

        /**
         *
         */
        abstract_reference &
        operator=
            ( bool value )
        {
            if ( value )
                current_block_ |= mask_;
            else
                current_block_ &= ~mask_;

            return *this;
        }

        /**
         *
         */
        template< typename OtherBlockType >
        bool
        operator==
            ( abstract_reference< OtherBlockType > const& o )
        { return static_cast< bool >( o ) == static_cast< bool >( *this ); }

        ///
        BlockType & current_block_;
        ///
        block_type const mask_;
    };

    ///
    using reference = abstract_reference< block_type >;

    ///
    using const_reference = abstract_reference< block_type const >;

    /**
     *  @brief Construct a null id.
     */
    id
        ( void )
            : blocks_{ }
    { }

    /**
     *  @brief Construct a random id.
     */
    explicit
    id
        ( std::default_random_engine & random_engine );

    /**
     *  @brief Construct an id from a string representation.
     */
    explicit
    id
        ( std::string value );

    /**
     *  @brief Construct an id by hashing a value.
     */
    explicit
    id
        ( value_to_hash_type const& value );

    /**
     *  @note From msb to lsb.
     */
    blocks_type::iterator
    begin
        ( void )
    { return blocks_.begin(); }

    /**
     *  @note From msb to lsb.
     */
    blocks_type::iterator
    end
        ( void )
    { return blocks_.end(); }

    /**
     *  @note From msb to lsb.
     */
    blocks_type::const_iterator
    begin
        ( void )
        const
    { return blocks_.begin(); }

    /**
     *  @note From msb to lsb.
     */
    blocks_type::const_iterator
    end
        ( void )
        const
    { return blocks_.end(); }

    /**
     *
     */
    bool
    operator==
        ( id const& o )
        const
    { return o.blocks_ == blocks_; }

    /**
     *
     */
    bool
    operator!=
        ( id const& o )
        const
    { return ! (o == *this); }

    /**
     *  @brief Return a const reference to a bit of the id.
     *  @param index The index of the bit (from 0 to BIT_SIZE - 1).
     *  @note Index 0 is the msb.
     */
    const_reference
    operator[]
        ( std::size_t index )
        const
    { return const_reference{ get_block( index ), get_mask( index ) }; }

    /**
     *  @brief Return a reference to a bit of the id.
     *  @param index The index of the bit (from 0 to BIT_SIZE - 1).
     *  @note Index 0 is the msb.
     */
    reference
    operator[]
        ( std::size_t index )
    { return reference{ get_block( index ), get_mask( index ) }; }

private:
    /**
     *
     */
    block_type &
    get_block
        ( std::size_t index )
    { return blocks_[ index / BIT_PER_BLOCK ]; }

    /**
     *
     */
    const block_type &
    get_block
        ( std::size_t index )
        const
    { return blocks_[ index / BIT_PER_BLOCK ]; }

    /**
     *
     */
    static block_type
    get_mask
        ( std::size_t index )
    { return 0x80 >> index % BIT_PER_BLOCK; }

private:
    ///
    blocks_type blocks_;
};

/**
 *
 */
inline bool
operator<
    ( id const& a
    , id const& b )
{
    return std::lexicographical_compare( a.begin(), a.end()
                                       , b.begin(), b.end() );
}

/**
 *
 */
std::ostream &
operator<<
    ( std::ostream & out
    , id const& i );

/**
 *
 */
inline id
distance
    ( id const& a
    , id const& b )
{
    id result;

    std::transform( a.begin(), a.end(), b.begin()
                  , result.begin()
                  , std::bit_xor< id::block_type >{} );

    return result;
}

} // namespace detail
} // namespace kademlia

#endif

