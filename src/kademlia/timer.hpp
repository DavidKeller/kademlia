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

#ifndef KADEMLIA_TIMER_HPP
#define KADEMLIA_TIMER_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <map>
#include <chrono>
#include <functional>
#include <boost/asio/io_service.hpp>
#include <boost/asio/basic_waitable_timer.hpp>

namespace kademlia {
namespace detail {

///
class timer final
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
    timer
        ( boost::asio::io_service & io_service );

    /**
     *
     */
    template< typename Callback >
    void
    expires_from_now
        ( duration const& timeout
        , Callback const& on_timer_expired );

private:
    ///
    using time_point = clock::time_point;

    ///
    using callback = std::function< void ( void ) >;

    ///
    using timeouts = std::multimap< time_point, callback >;

    ///
    using deadline_timer = boost::asio::basic_waitable_timer< clock >;

private:
    /**
     *
     */
    void
    schedule_next_tick
        ( time_point const& expiration_time );

    /**
     *
     */
    void
    on_fire
        ( boost::system::error_code const& failure );

private:
    ///
    deadline_timer timer_;
    ///
    timeouts timeouts_;
};

template< typename Callback >
void
timer::expires_from_now
    ( duration const& timeout
    , Callback const& on_timer_expired )
{
    auto expiration_time = clock::now() + timeout;

    // If the current expiration time will be the sooner to expires
    // then cancel any pending wait and schedule this one instead.
    if ( timeouts_.empty() || expiration_time < timeouts_.begin()->first )
        schedule_next_tick( expiration_time );

    timeouts_.emplace( expiration_time, on_timer_expired );
}

} // namespace detail
} // namespace kademlia

#endif

