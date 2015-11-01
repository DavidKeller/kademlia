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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <utility>
#include <vector>
#include <boost/iterator/iterator_facade.hpp>

#include <kademlia/detail/cxx11_macros.hpp>

#include "kademlia/id.hpp"
#include "kademlia/log.hpp"

namespace kademlia {
namespace detail {

/**
 *  This class keeps track of peers and find the known peer closed to an id.
 *  @note Current implementation use a discret symbol approach.
 */
template< typename PeerType >
class routing_table final
{
public:
    ///
    enum { DEFAULT_K_BUCKET_SIZE = 20 };

    ///
    using peer_type = PeerType;

    ///
    using value_type = std::pair< id, peer_type >;

    class iterator;

public:
    /**
     *  Construct the routing_table implementation.
     */
    routing_table
        ( id const& my_id
        , std::size_t k_bucket_size = DEFAULT_K_BUCKET_SIZE )
            : k_buckets_( id::BIT_SIZE ), my_id_( my_id )
            , peer_count_( 0 ), k_bucket_size_( k_bucket_size )
            , largest_k_bucket_index_( 0 )
    {
        assert( k_bucket_size_ > 0 && "k_bucket size must be > 0" );

        LOG_DEBUG( routing_table, this ) << "created with id '"
                << my_id_ << "'." << std::endl;
    }

    /**
     *  Disabled copy constructor.
     */
    routing_table
        ( routing_table const& )
        = delete;

    /**
     *  Disabled assignement operator.
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
        const
    { return peer_count_; }

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
        , peer_type const& new_peer )
    {
        LOG_DEBUG( routing_table, this ) << "pushing peer '"
                << new_peer << "' as '"
                << peer_id << "'." << std::endl;

        auto k_bucket_index = find_k_bucket_index( peer_id );
        auto & bucket = k_buckets_[ k_bucket_index ];

        // If there is room in the bucket.
        if ( bucket.size() == k_bucket_size_ )
        {
            update_largest_k_bucket_index( k_bucket_index );

            if ( k_bucket_index != largest_k_bucket_index_ )
                return false;
        }

        auto const end = bucket.end();

        // Check if the peer is not already known.
        auto is_peer_known = [ &peer_id ] ( value_type const& entry )
        { return entry.first == peer_id; };

        if ( std::find_if( bucket.begin(), end, is_peer_known ) != end )
            return false;

        bucket.insert( end, value_type{ peer_id, new_peer } );
        ++ peer_count_;

        return true;
    }

    /**
     *  Remove a peer from the routing table.
     *  @return true if the peer has been removed.
     *  @note Complexity: O(log n)
     */
    bool
    remove
        ( id const& peer_id )
    {
        LOG_DEBUG( routing_table, this ) << "removing peer '"
                << peer_id << "'." << std::endl;

        // Find the closer bucket.
        auto & bucket = k_buckets_[ find_k_bucket_index( peer_id ) ];

        // Check if the peer is inside.
        auto is_peer_known = [&peer_id] ( value_type const& entry )
        { return entry.first == peer_id; };

        auto i = std::find_if( bucket.begin(), bucket.end(), is_peer_known );

        // If the peer wasn't inside.
        if ( i == bucket.end() )
            return false;

        // Remove it.
        bucket.erase( i );
        -- peer_count_;

        return true;
    }

    /**
     *  Find closest peers to an id.
     *  @return An iterator to the closest peer from the id to the far.
     *  @note Complexity: O(log n)
     */
    iterator
    find
        ( id const& id_to_find )
    {
        LOG_DEBUG( routing_table, this ) << "finding peer near '"
                << id_to_find << "'." << std::endl;

        auto index = std::max( get_lowest_k_bucket_index()
                             , find_k_bucket_index( id_to_find ) );

        auto i = std::next( k_buckets_.begin(), index );

        // Find the first non empty k_bucket.
        while ( i->empty() && i != k_buckets_.begin() )
            -- i;

        return iterator( &k_buckets_, i, i->begin() );
    }

    /**
     *  @return An iterator to the end of the routing table.
     */
    iterator
    end
        ( void )
    {
        assert( k_buckets_.size() > 0
              && "routing_table must always contains k_buckets" );
        auto const first_k_bucket = k_buckets_.begin();

        return iterator( &k_buckets_, first_k_bucket, first_k_bucket->end() );
    }

    /**
     *  Print the routing table content.
     *  @param out The output stream.
     *  @param table The routing table to print.
     *  @return A reference to the output stream.
     */
    friend std::ostream &
    operator<<
        ( std::ostream & out
        , routing_table const& table )
    {
        out << "{" << std::endl
            << "\t\"id\": " << table.my_id_ << "," << std::endl
            << "\t\"peer_count\": " << table.peer_count_ << ',' << std::endl
            << "\t\"k_bucket_size\": " << table.k_bucket_size_<< ',' << std::endl
            << "\t\"k_buckets\": " << std::endl;

        for ( std::size_t i = 0, e = table.k_buckets_.size(); i != e; ++i )
        {
            out << "\t{" << std::endl
                << "\t\t\"index\": " << i << "," << std::endl
                << "\t\t\"bit_value\": " << bool(table.my_id_[i]) << "," << std::endl
                << "\t\t\"peer_count\": " << table.k_buckets_[i].size() << std::endl
                << "\t}" << std::endl;
        }

        return out << "}" << std::endl;
    }

private:
    /// Contains peer with a common base id.
    using k_bucket = std::list< value_type >;
    /// Contains all the k_bucket.
    /// @note Algorithms expect a vector here, do not change this.
    using k_buckets = std::vector< k_bucket >;

private:
    /**
     *
     */
    std::size_t
    find_k_bucket_index
        ( id const& id_to_find )
        const
    {
        // Find closest bucket from the peer id.
        // i.e. the index of the first different bit
        // in the id of the new peer vs our id is equal to the
        // index of the closest bucket in the buckets container.
        std::size_t bit_index = 0;
        while ( bit_index < id::BIT_SIZE - 1
              && id_to_find[ bit_index ] == my_id_[ bit_index ] )
            ++ bit_index;

        LOG_DEBUG( routing_table, this ) << "found bucket at index '"
                << bit_index << "'." << std::endl;

        return bit_index;
    }

    /**
     *
     */
    std::size_t
    get_lowest_k_bucket_index
        ( void )
        const
    {
        std::size_t i = 0ULL, e = k_buckets_.size() - 1;

        for ( std::size_t peer_count = 0ULL
            ; i != e && peer_count <= k_bucket_size_
            ; ++ i )
            peer_count += k_buckets_[ i ].size();

        LOG_DEBUG( routing_table, this ) << "bottom bucket is at index '"
                << i << "'." << std::endl;

        return i;
    }

    /**
     *
     */
    void
    update_largest_k_bucket_index
        ( std::size_t index )
    {
        if ( k_buckets_[ largest_k_bucket_index_ ].size() <= k_bucket_size_ )
            largest_k_bucket_index_ = index;
    }

private:
    /// This contains buckets up to id bit count.
    k_buckets k_buckets_;
    /// Own id.
    id const my_id_;
    /// Keep a track of peer count to make size() complexity O(1).
    std::size_t peer_count_;
    /// This is max number of peers stored per k_bucket.
    std::size_t k_bucket_size_;
    /// This keeps the index of the largest subtree.
    std::size_t largest_k_bucket_index_;
};

/**
 *
 */
template< typename PeerType >
class routing_table< PeerType >::iterator
    : public boost::iterator_facade
        < iterator
        , typename routing_table::value_type
        , boost::single_pass_traversal_tag >
{
public:
    /**
     *
     */
    iterator
        ( k_buckets * buckets
        , typename k_buckets::iterator current_bucket
        , typename k_bucket::iterator current_peer )
        : k_buckets_( buckets )
        , current_k_bucket_( current_bucket )
        , current_entry_( current_peer )
    { }

    /**
     *
     */
    iterator &
    operator=
        ( iterator const& o )
    {
        k_buckets_ = o.k_buckets_;
        current_k_bucket_ = o.current_k_bucket_;
        current_entry_ = o.current_entry_;

        return *this;
    }

private:
    friend class boost::iterator_core_access;

    /**
     *
     */
    void
    increment
        ( void )
    {
        ++ current_entry_;

        // If the current entry is not at the end of the bucket
        // then there is nothing more to do.
        if ( current_entry_ != current_k_bucket_->end() )
            return;

        // If the current bucket is already the first (far)
        // then there is nothing more to do, we reach the end of the routing table.
        if ( current_k_bucket_ == k_buckets_->begin() )
            return;

        // Go to the next non-empty bucket and start from its first entry.
        do
            -- current_k_bucket_;
        while ( current_k_bucket_->empty() && current_k_bucket_ != k_buckets_->begin() );
        current_entry_ = current_k_bucket_->begin();
    }

    /**
     *
     */
    bool
    equal
        ( iterator const& o )
        const
    {
        return k_buckets_ == o.k_buckets_
                && current_k_bucket_ == o.current_k_bucket_
                && current_entry_ == o.current_entry_;
    }

    /**
     *
     */
    typename routing_table::value_type &
    dereference
        ( void )
        const
    { return *current_entry_; }

private:
    ///
    k_buckets * k_buckets_;
    ///
    typename k_buckets::iterator current_k_bucket_;
    ///
    typename k_bucket::iterator current_entry_;

};

} // namespace detail
} // namespace kademlia

#endif

