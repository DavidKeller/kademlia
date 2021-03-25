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

#ifndef KADEMLIA_R_HPP
#define KADEMLIA_R_HPP

#ifdef _MSC_VER
#   pragma once
#   pragma warning( push )
#   pragma warning( disable : 4521 )
#endif

#include <system_error>
#include <utility>
#include <type_traits>

#include <kademlia/detail/cxx11_macros.hpp>

namespace kademlia {
namespace detail {

/**
 *  @brief Designed to return a value <b>or</b> an error code.
 *  @tparam ReturnType The value type to return if no error occurred.
 */
template< typename ReturnType >
class r
{
public:
    /// Exception type thrown when a value is accessed
    /// while the r contains an error.
    using exception_type = std::system_error;

    /// The error type used to store error.
    using error_type = std::error_code;

    /// The value stored when no error occured.
    using value_type = ReturnType;

public:
    /**
     *  @brief Construct this initialized with
     *         a value whose emplace constructor
     *         arguments are provided
     *         to this constructor.
     *  @tparam Args Value constructor arguments type.
     *  @param args Value constructor arguments.
     */
    template< typename ...Args >
    r
        ( Args &&... args)
        : error_{ }
    { construct_value( std::forward< Args >( args )... ); }

    /**
     *  @brief Construct this initialized with an error.
     *  @param error A constant reference to an error.
     */
    r
        ( error_type const & error )
        : error_{ error }
    { }

    /**
     *  @brief Construct this initialized with an error.
     *  @param error A mutable reference to an error.
     */
    r
        ( error_type & error )
        : r{ const_cast< error_type const & >( error ) }
    { }

    /**
     *  @brief Construct this initialized with an error.
     *  @param error A rvalue reference to an error.
     */
    r
        ( error_type && error )
        : r{ const_cast< error_type const & >( error ) }
    { }

    /**
     *  @brief Copy constructor.
     *  @param other A constant reference to an error.
     */
    r
        ( r const & other )
        : error_{ other.error_ }
    { if ( other ) construct_value( other.v() ); }

    /**
     *  @brief Copy constructor.
     *  @param other A mutable reference to an error.
     */
    r
        ( r & other )
        : r{ const_cast< r const & >( other ) }
    { }

    /**
     *  @brief Copy constructor.
     *  @param other A rvalue reference to an error.
     */
    r
        ( r && other )
        : error_{ std::move( other.error_ ) }
    { if ( other ) construct_value( std::move( other.v() ) ); }

    /**
     *  @brief Destructor.
     */
    ~r
        ( void )
    { destruct_value_if_present(); }

    /**
     *  @brief Assignment operator.
     *  @param other A constant reference to an error.
     */
    r &
    operator=
        ( r const & other )
    { *this = r{ other }; }

    /**
     *  @brief Assignment operator.
     *  @param other A rvalue reference to an error.
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
     *  @brief Assignment from an error operator.
     *  @param error The error to assign to this.
     *  @return A reference to this.
     */
    r &
    operator=
        ( error_type const & error )
    {
        assert( error && "unexpected success error code" );
        destruct_value_if_present();
        error_ = error;

        return *this;
    }

    /**
     *  @brief Assignment from a value operator.
     *  @param value The value to assign to this.
     *  @return A reference to this.
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
     *  @brief Assignment from a value operator.
     *  @param value The value to assign to this.
     *  @return A reference to this.
     */
    r &
    operator=
        ( value_type && value )
    {
        destruct_value_if_present();
        error_.clear();
        construct_value( std::move( value ) );

        return *this;
    }

    /**
     *  @brief bool operator used to check if this
     *         has been initialized with a value.
     *  @return true if initliazed from a value,
     *          false otherwise.
     */
    explicit
    operator bool() const CXX11_NOEXCEPT
    { return ! error_; }

    /**
     *  @brief Get the value stored in this.
     *  @return A reference to the stored value.
     *  @note throw exception_type if this is initiliazed with an error.
     *  @throw exception_type
     */
    value_type &
    v()
    { return access_value_or_throw(); }

    /**
     *  @copydoc value_type & v()
     */
    value_type const &
    v() const
    { return access_value_or_throw(); }

    /**
     *  @brief Get the error stored in this.
     *  @return A constant reference to the stored error.
     */
    error_type const &
    e() const
    { return error_; }

private:
    /**
     *  @brief Construct the value in the initilized value_ buffer.
     */
    template< typename ...Args >
    void
    construct_value
        ( Args &&... args )
    { ::new(&value_) value_type( std::forward< Args >( args )... ); }

    /**
     *  @brief If a value has been stored in the value_ buffer,
     *         call the destructor.
     */
    void
    destruct_value_if_present()
    { if (*this) access_value_or_throw().~ReturnType(); }

    /**
     *  @brief Return the value_ buffer as a value.
     */
    value_type &
    access_value_or_throw()
    {
        if (! *this)
            throw exception_type{ error_ };

        return reinterpret_cast< value_type & >( value_ );
    }

    /**
     *  @copydoc value_type & access_value_or_throw()
     */
    value_type const &
    access_value_or_throw() const
    {
        if (! *this)
            throw exception_type{ error_ };

        return reinterpret_cast< value_type const & >( value_ );
    }


private:
    /// The error stored if no value has be set.
    error_type error_;

    /// A buffer used to store a value if this is not
    /// initliazed with an error.
    typename std::aligned_storage< sizeof( value_type )
                                 , std::alignment_of< value_type >::value
                                 >::type value_;
};

} // namespace detail
} // namespace kademlia

#ifdef _MSC_VER
#   pragma warning( pop )
#endif

#endif

