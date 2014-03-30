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


#include "routing_table.hpp"

#include <cstdint>
#include <iostream>
#include <cassert>
#include <algorithm>

namespace kademlia {
namespace detail {

routing_table::routing_table
    ( id const& my_id
    , std::size_t k_bucket_size )
    : k_buckets_( id::BIT_SIZE ), my_id_( my_id )
    , peer_count_( 0 ), k_bucket_size_( k_bucket_size )
{
    assert( k_bucket_size_ > 0 && "k_bucket size must be > 0" );
}

routing_table::~routing_table
    ( void )
{ }

std::size_t
routing_table::peer_count
    ( void )
    const
{ return peer_count_; }

bool
routing_table::push
    ( id const& peer_id
    , message_socket::endpoint_type const& new_peer )
{
    auto const target_k_bucket = find_closest_k_bucket( peer_id );
        
    // If there is room in the bucket.
    if ( target_k_bucket->size() == k_bucket_size_ ) 
        return false;
        
    auto const end = target_k_bucket->end();
    
    // Check if the peer is not already known.
    if ( std::find( target_k_bucket->begin(), end, peer_id ) != end )
        return false;
    
    target_k_bucket->insert( end, value_type( peer_id, new_peer ) ); 
    ++ peer_count_;
    
    return true;
}

bool
operator==
    ( routing_table::value_type const& entry 
    , id const& id_to_match )
{
    return entry.first == id_to_match;
}
    
bool
routing_table::remove
    ( id const& peer_id )
{
    // Find the closer bucket.
    auto bucket = find_closest_k_bucket( peer_id );
    
    // Check if the peer is inside.
    auto i = std::find( bucket->begin(), bucket->end(), peer_id );
    
    // The peer wasn't inside.
    if ( i == bucket->end() )
        return false;
    
    // Remove it.
    bucket->erase( i );
    -- peer_count_;
    
    return true;
}

routing_table::iterator
routing_table::find
    ( id const& id_to_find )
{
    auto i = find_closest_k_bucket( id_to_find );

    // Find the first non empty k_bucket.
    while( i->empty() && i != k_buckets_.begin() )
        -- i;

    return iterator( &k_buckets_, i, i->begin() );
}

routing_table::k_buckets::iterator
routing_table::find_closest_k_bucket
    ( id const& id_to_find )
{
    // Start from the far bucket.
    k_buckets::iterator current_k_bucket = k_buckets_.begin();
    
    std::size_t bit_index = 0; 
    std::size_t const last_bit_index = id::BIT_SIZE - 1;
    // Find closest bucket from the peer id.
    // i.e. the index of the first different bit
    // in the id of the new peer vs our id is equal to the
    // index of the closest bucket in the buckets container.
    while ( bit_index < last_bit_index 
          && id_to_find[ bit_index ] == my_id_[ bit_index ] )
    {
        ++ bit_index;
        ++ current_k_bucket;
    }

    return current_k_bucket;
}

routing_table::iterator
routing_table::end
    ( void )
{ 
    assert( k_buckets_.size() > 0 
          && "routing_table must always contains k_buckets" );
    auto const first_k_bucket = k_buckets_.begin();
    
    return iterator( &k_buckets_, first_k_bucket, first_k_bucket->end() ); 
}

routing_table::iterator::iterator
    ( k_buckets * buckets
    , k_buckets::iterator current_bucket
    , k_bucket::iterator current_peer )
    : k_buckets_( buckets )
    , current_k_bucket_( current_bucket )
    , current_entry_( current_peer )
{ }

routing_table::iterator &
routing_table::iterator::operator=
    ( iterator const& o )
{
    k_buckets_ = o.k_buckets_;
    current_k_bucket_ = o.current_k_bucket_;
    current_entry_ = o.current_entry_;

    return *this;
}

void
routing_table::iterator::increment
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
       
bool
routing_table::iterator::equal
    ( iterator const& o )
    const
{ 
    return k_buckets_ == o.k_buckets_
            && current_k_bucket_ == o.current_k_bucket_
            && current_entry_ == o.current_entry_;
}

routing_table::value_type & 
routing_table::iterator::dereference
    ( void )
    const
{ return *current_entry_; }

std::ostream &
operator<<
    ( std::ostream & out
    , routing_table const& table )
{
    out << "{" << std::endl
        << "\t\"id\": " << table.my_id_ << "," << std::endl
        << "\t\"peer_count\": " << table.peer_count_ << ',' << std::endl
        << "\t\"k_bucket_size\": " << table.k_bucket_size_<< ',' << std::endl
        << "\t\"k_buckets\": " << std::endl;

    
    for ( std::uint16_t i = 0; i < table.k_buckets_.size(); ++i )
    {
        out << "\t{" << std::endl
            << "\t\t\"index\": " << i << "," << std::endl
            << "\t\t\"bit_value\": " << table.my_id_[i] << "," << std::endl
            << "\t\t\"peer_count\": " << table.k_buckets_[i].size() << std::endl
            << "\t}" << std::endl;
    }

    return out;    
}

} // namespace detail
} // namespace kademlia
