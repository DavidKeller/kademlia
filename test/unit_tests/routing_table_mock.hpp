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

#ifndef KADEMLIA_TEST_HELPERS_ROUTING_TABLE_MOCK_HPP
#define KADEMLIA_TEST_HELPERS_ROUTING_TABLE_MOCK_HPP

#include <vector>
#include <deque>
#include <utility>
#include <stdexcept>

#include "kademlia/message.hpp"
#include "kademlia/ip_endpoint.hpp"
#include "kademlia/peer.hpp"

namespace kademlia {
namespace test {

struct routing_table_mock
{
    using peer_type = std::pair< detail::id, detail::ip_endpoint >;
    using peers_type = std::vector< peer_type >;
    using expected_ids_type = std::deque< detail::id >;

    using iterator_type = peers_type::iterator;

    routing_table_mock
        ( void )
        : expected_ids_()
        , peers_()
        , find_call_count_()
    { }

    iterator_type
    find
        ( detail::id const& id )
    {
        if ( expected_ids_.empty() || id != expected_ids_.front() )
            throw std::runtime_error( "Unexpected searched id." );

        expected_ids_.pop_front();

        ++ find_call_count_;

        return peers_.begin();
    }

    void
    push
        ( detail::id const& id
        , detail::ip_endpoint const& endpoint )
    { peers_.emplace_back( id, endpoint ); }

    iterator_type
    end
        ( void )
    { return peers_.end(); }

    expected_ids_type expected_ids_;
    peers_type peers_;
    uint64_t find_call_count_;
};

} // namespace test
} // namespace kademlia

#endif // KADEMLIA_TEST_HELPERS_ROUTING_TABLE_MOCK_HPP

