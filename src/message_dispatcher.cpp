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

#include "message_dispatcher.hpp"

#include <cassert>
#include <algorithm>

#include <kademlia/error.hpp>

namespace kademlia {
namespace detail {

message_dispatcher::message_dispatcher
    ( boost::asio::io_service & io_service )
    : timer_{ io_service }
    , timeouts_{}
    , associations_{}
{ }

void
message_dispatcher::associate_message_with_task_for
    ( id const& message_id
    , task_base * task
    , duration const& timeout )
{
    auto i = associations_.emplace( message_id, task ); 
    assert( i.second && "an id can't be registered twice" );

    auto expiration_time = clock::now() + timeout;

    while ( ! timeouts_.emplace( expiration_time, message_id ).second )
        // If an equivalent expiration exists, 
        // modify the current to make it unique.
        expiration_time += std::chrono::nanoseconds( 1 ); 
    
    // If the current expiration time will be the sooner to expires
    // then cancel any pending wait and schedule this one instead.
    if ( timeouts_.begin()->first == expiration_time )
        schedule_next_tick( expiration_time );
}

std::error_code
message_dispatcher::dispatch_message
    ( header const& h
    , buffer::const_iterator i
    , buffer::const_iterator e )
{
    auto task = pop_association( h.random_token_ );
    if ( task ) 
        return task->handle_message( h, i, e );

    return make_error_code( UNASSOCIATED_MESSAGE_ID ); 
}

task_base *
message_dispatcher::pop_association
    ( id const& message_id )
{
    task_base * task = nullptr;

    auto i = associations_.find( message_id );
    if ( i != associations_.end() )
    {
        task = i->second;
        associations_.erase( i );
    }

    return task;
}

void
message_dispatcher::schedule_next_tick
    ( time_point const& expiration_time )
{
    timer_.expires_at( expiration_time );

    auto fire = [ this ]( boost::system::error_code const& failure ) 
    { 
        if ( failure )
            return;

        // The current expired association is the lowest
        // in the map, so remove it.
        auto const expired_id = timeouts_.begin()->second;
        timeouts_.erase( timeouts_.begin() );

        // If there is a remaining timeout, schedule it.
        if ( ! timeouts_.empty() ) 
            schedule_next_tick( timeouts_.begin()->first );
        
        auto task = pop_association( expired_id );
        // If the association has not already been
        // removed by a call to handle_message.
        if ( task ) 
            task->handle_message_timeout( expired_id );
    };

    timer_.async_wait( fire );
}

std::size_t
message_dispatcher::count_associations
    ( void ) 
    const
{ return associations_.size(); }

} // namespace detail
} // namespace kademlia

