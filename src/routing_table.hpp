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

#ifndef KADEMLIA_ROUTING_TABLE_HPP
#define KADEMLIA_ROUTING_TABLE_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <utility>
#include <list>
#include <vector>
#include <iosfwd>
#include <boost/iterator/iterator_facade.hpp>

#include <kademlia/detail/cxx11_macros.hpp>

#include "id.hpp"
#include "message_socket.hpp"

namespace kademlia {
namespace detail {

/**
 *  This class keeps track of peers and find the known peer closed to an id.
 *  @note Current implementation use a discret symbol approach.
 */
class routing_table final
{
public:
    enum { DEFAULT_K_BUCKET_SIZE = 20 };

    class iterator;
    
    using value_type = std::pair< id, message_socket::endpoint_type >;
    
public:
    /**
     *  Construct the routing_table implementation.
     */
    routing_table
        ( id const& my_id
        , std::size_t k_bucket_size = DEFAULT_K_BUCKET_SIZE );

    /**
     *  Destroy the routing_table implementation.
     */
    ~routing_table
        ( void );

    /**
     * Disabled copy constructor.
     */
    routing_table
        ( routing_table const& )
        = delete;

    /**
     * Disabled assignement operator.
     */
    routing_table&
    operator=
        ( routing_table const& )
        = delete;

    /**
     *  Count the number of peer in the routing table.
     *  @note Complexity: O(1).
     */
    std::size_t 
    peer_count
        ( void )
        const;

    /**
     *  Register a peer into the routing table.
     *  @return true if the peer has been inserted.
     *  @note This method takes ownership of the peer.
     *  @note The peer may not be pushed if the target bucket is full.
     *  @note Complexity: O(log n)
     */
    bool
    push
        ( id const& peer_id
        , message_socket::endpoint_type const& new_peer );
    
    /**
     *  Remove a peer from the routing table.
     *  @return true if the peer has been removed.
     *  @note Complexity: O(log n)
     */
    bool
    remove
        ( id const& peer_id );

    /**
     *  Find closest peers to an id.
     *  @return An iterator to the closest peer from the id to the far.
     *  @note Complexity: O(log n)
     */
    iterator
    find
        ( id const& id_to_find );
    
    /**
     *  @return An iterator to the end of the routing table.
     */
    iterator
    end
        ( void );

    /**
     *  Print the routing table content.
     *  @param out The output stream.
     *  @param table The routing table to print.
     *  @return A reference to the output stream.
     */
    friend std::ostream &
    operator<<
        ( std::ostream & out
        , routing_table const& table );

private:
    
    /// Contains peer with a common base id.
    using k_bucket = std::list< value_type >;
    /// Contains all the k_bucket.
    /// @note Algorithms expect a vector here, do not change this.
    using k_buckets = std::vector< k_bucket >;

private:
    k_buckets::iterator
    find_closest_k_bucket
        ( id const& id_to_find );
    
    friend bool
    operator==
        ( value_type const& entry 
        , id const& id_to_match );

private:
    /// This contains buckets up to id bit count.
    k_buckets k_buckets_;
    /// Own id.
    id const my_id_;
    /// Keep a track of peer count to make size() complexity O(1).
    std::size_t peer_count_;
    /// This is max number of peers stored per k_bucket.
    std::size_t k_bucket_size_;
};

/**
 * 
 */
class routing_table::iterator
    : public boost::iterator_facade
        < iterator
        , routing_table::value_type
        , boost::single_pass_traversal_tag >
{
public:
    /**
     *
     */
    iterator
        ( k_buckets * buckets
        , k_buckets::iterator current_bucket
        , k_bucket::iterator current_peer );

    /**
     *
     */
    iterator &
    operator=
        ( iterator const& o );

private:
    friend class boost::iterator_core_access;

    /**
     *
     */
    void
    increment
        ( void );
       
    /**
     *
     */
    bool
    equal
        ( iterator const& o )
        const;

    /**
     *
     */
    routing_table::value_type & 
    dereference
        ( void )
        const;

private:
    ///
    k_buckets * k_buckets_;
    ///
    k_buckets::iterator current_k_bucket_;
    ///
    k_bucket::iterator current_entry_;

};

} // namespace detail
} // namespace kademlia

#endif

