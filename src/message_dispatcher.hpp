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

#ifndef KADEMLIA_MESSAGE_DISPATCHER_HPP
#define KADEMLIA_MESSAGE_DISPATCHER_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <map>
#include <chrono>
#include <system_error>
#include <boost/asio/io_service.hpp>
#include <boost/asio/basic_waitable_timer.hpp>

#include "id.hpp"
#include "task_base.hpp"

namespace kademlia {
namespace detail {

///
class message_dispatcher final
{
public:
    ///
    using clock = std::chrono::steady_clock;

    ///
    using duration = clock::duration;

public:
    /**
     *
     */
    explicit 
    message_dispatcher
        ( boost::asio::io_service & io_service );

    /**
     *
     */
    void
    associate_message_with_task_for
        ( id const& message_id
        , task_base * task
        , duration const& timeout );

    /**
     *
     */
    std::error_code
    dispatch_message
        ( header const& h
        , buffer::const_iterator i
        , buffer::const_iterator e );

private:
    /// 
    using time_point = clock::time_point;

    ///
    using timeouts = std::map< time_point, id >;

    ///
    using associations = std::map< id, task_base * >;

    /// 
    using timer = boost::asio::basic_waitable_timer< clock >;

private:
    /**
     *
     */
    task_base *
    pop_association
        ( id const& request_id );

    /**
     *
     */
    void
    schedule_next_tick
        ( time_point const& expiration_time );

    /**
     *
     */
    std::size_t
    count_associations
        ( void ) 
        const;

private:
    ///
    timer timer_;
    ///
    timeouts timeouts_; 
    ///
    associations associations_;
};

} // namespace detail
} // namespace kademlia

#endif

