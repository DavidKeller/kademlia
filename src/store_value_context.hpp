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

#ifndef KADEMLIA_STORE_VALUE_CONTEXT_HPP
#define KADEMLIA_STORE_VALUE_CONTEXT_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <system_error>

#include "value_context.hpp"

namespace kademlia {
namespace detail {

///
template< typename SaveHandlerType, typename DataType >
class store_value_context final
    : public value_context
{
public:
    ///
    using save_handler_type = SaveHandlerType;

    ///
    using data_type = DataType;

public:
    /**
     *
     */
    template< typename Iterator, typename HandlerType >
    store_value_context( detail::id const & key
                       , data_type const& data
                       , Iterator i, Iterator e
                       , HandlerType && save_handler );

    /**
     *
     */
    void
    notify_caller
        ( std::error_code const& failure );

    /**
     *
     */
    data_type const&
    get_data
        ( void )
        const;

private:
    ///
    data_type data_;
    ///
    save_handler_type save_handler_;
};

template< typename SaveHandlerType, typename DataType >
template< typename Iterator, typename HandlerType >
inline
store_value_context< SaveHandlerType, DataType >::store_value_context
    ( detail::id const & key
    , data_type const& data
    , Iterator i, Iterator e
    , HandlerType && save_handler )
        : value_context{ key, i, e }
        , data_{ data }
        , save_handler_( std::forward< HandlerType >( save_handler ) )
{ }

template< typename SaveHandlerType, typename DataType >
inline void
store_value_context< SaveHandlerType, DataType >::notify_caller
    ( std::error_code const& failure )
{ save_handler_( failure ); }

template< typename SaveHandlerType, typename DataType >
inline typename store_value_context< SaveHandlerType, DataType >::data_type const&
store_value_context< SaveHandlerType, DataType >::get_data
    ( void )
    const
{ return data_; }

} // namespace detail
} // namespace kademlia

#endif

