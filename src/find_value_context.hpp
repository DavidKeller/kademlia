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

#ifndef KADEMLIA_FIND_VALUE_CONTEXT_HPP
#define KADEMLIA_FIND_VALUE_CONTEXT_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <system_error>

#include "value_context.hpp"

namespace kademlia {
namespace detail {

///
template< typename LoadHandlerType, typename DataType >
class find_value_context final
    : public value_context
{
public:
    ///
    using load_handler_type = LoadHandlerType;

    ///
    using data_type = DataType;

public:
    /**
     *
     */
    template< typename Iterator, typename HandlerType >
    find_value_context
        ( id const & searched_key
        , Iterator i, Iterator e
        , HandlerType && load_handler )
        : value_context( searched_key, i, e )
        , load_handler_{ std::forward< HandlerType >( load_handler ) }
        , is_finished_{}
    { }

    /**
     *
     */
    void
    notify_caller
        ( data_type const& data )
    { 
        load_handler_( std::error_code{}, data ); 
        is_finished_ = true;
    }

    /**
     *
     */
    void
    notify_caller
        ( std::error_code const& failure )
    { 
        load_handler_( failure, data_type{} ); 
        is_finished_ = true;
    }

    /**
     *
     */
    bool
    is_caller_notified
        ( void )
        const
    { return is_finished_; }

private:
    ///
    load_handler_type load_handler_;
    ///
    bool is_finished_;
};

} // namespace detail
} // namespace kademlia

#endif

