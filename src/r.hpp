// Copyright (c) 2013, David Keller
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

#ifndef KADEMLIA_R_HPP
#define KADEMLIA_R_HPP

#if defined(_MSC_VER)
#   pragma once
#endif

namespace kademlia {
namespace detail {

#include <system_error>
#include <utility>
#include <type_traits>

/**
 *
 */
template< typename ReturnType >
class r
{
public:
    ///
    using exception_type = std::system_error;

    ///
    using error_type = std::error_code;

    ///
    using value_type = ReturnType;

public:
    /**
     *
     */
    template< typename ...Args >
    r
        ( Args &&... args) 
        : error_{ }
    { construct_value( args... ); }

    /**
     *
     */
    r
        ( error_type const & error )
        : error_{ error } 
    { } 

    /**
     *
     */
    r
        ( error_type & error )
        : r{ const_cast< error_type const & >( error ) } 
    { } 

    /**
     *
     */
    r
        ( error_type && error )
        : r{ const_cast< error_type const & >( error ) } 
    { } 

    /**
     *
     */
    r
        ( r const & other )
        : error_{ other.error_ }
    { if ( other ) construct_value( other.v() ); }

    /**
     *
     */
    r
        ( r & other )
        : r{ const_cast< r const & >( other ) }
    { }

    /**
     *
     */
    r
        ( r && other )
        : error_{ std::move( other.error_ ) }
    { if ( other ) construct_value( std::move( other.v() ) ); }

    /**
     *
     */
    ~r
        ( void )
    { destruct_value_if_present(); }

    /**
     *
     */
    r &
    operator=
        ( r const & other )
    { *this = r{ other }; }

    /**
     *
     */
    r &
    operator=
        ( r && other )
    { 
        destruct_value_if_present();
        error_ = other.error_;

        if ( other ) construct_value( std::move( other.v() ) );
    }

    /**
     *
     */
    r &
    operator=
        ( error_type const & error )
    { 
        assert( error && "error must be initialized" );
        destruct_value_if_present();
        error_ = error;

        return *this;
    }

    /**
     *
     */
    r &
    operator=
        ( value_type const & value )
    { 
        destruct_value_if_present();
        error_.clear();
        construct_value( value );

        return *this;
    }

    /**
     *
     */
    r &
    operator=
        ( value_type && value )
    { 
        destruct_value_if_present();
        error_.clear();
        construct_value( std::forward< value_type >( value ) );

        return *this;
    }


    /**
     *
     */
    explicit 
    operator bool() const noexcept
    { return ! error_; }

    /**
     *
     */
    value_type &
    v() 
    { return access_value_or_throw(); }

    /**
     *
     */
    value_type const &
    v() const
    { return access_value_or_throw(); }

    /**
     *
     */
    error_type const &
    e() const
    { return error_; }

private:
    /**
     *
     */
    template< typename ...Args >
    void
    construct_value 
        ( Args &&... args )
    { ::new(&value_) value_type( std::forward< Args >( args )... ); }

    /**
     *
     */
    void
    destruct_value_if_present()
    { if (*this) access_value_or_throw().~ReturnType(); }

    /**
     *
     */
    value_type &
    access_value_or_throw()
    { 
        if (! *this) 
            throw exception_type{ error_ };

        return reinterpret_cast< value_type & >( value_ ); 
    }

    /**
     *
     */
    value_type const &
    access_value_or_throw() const
    { 
        if (! *this) 
            throw exception_type{ error_ };

        return reinterpret_cast< value_type const & >( value_ ); 
    }


private:
    ///
    error_type error_;

    ///
    typename std::aligned_storage< sizeof( value_type )
                                 , std::alignment_of< value_type >::value
                                 >::type value_;
};

} // namespace detail
} // namespace kademlia

#endif

