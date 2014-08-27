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

#include "kademlia/response_dispatcher.hpp"

#include <cassert>

#include <kademlia/error.hpp>

namespace kademlia {
namespace detail {

void
response_dispatcher::push_association
    ( id const& message_id
    , callback const& on_message_received )
{
    auto i = associations_.emplace( message_id, on_message_received );
    assert( i.second && "an id can't be registered twice" );
}

bool
response_dispatcher::remove_association
    ( id const& message_id )
{ return associations_.erase( message_id ) > 0; }

std::error_code
response_dispatcher::dispatch_message
    ( endpoint_type const& sender
    , header const& h
    , buffer::const_iterator i
    , buffer::const_iterator e )
{
    auto association = associations_.find( h.random_token_ );
    if ( association == associations_.end() )
        return make_error_code( UNASSOCIATED_MESSAGE_ID );

    association->second( sender, h, i, e );
    associations_.erase( association );

    return std::error_code{};
}

} // namespace detail
} // namespace kademlia

