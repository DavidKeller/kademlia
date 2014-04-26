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

#ifndef KADEMLIA_VALUE_CONTEXT_HPP
#define KADEMLIA_VALUE_CONTEXT_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <cassert>
#include <map>
#include <vector>

#include "candidate.hpp"

namespace kademlia {
namespace detail {

///
class value_context
{
protected:
    ~value_context
        ( void )
    { }

    template< typename Iterator >
    explicit
    value_context( id const & key
                , Iterator i, Iterator e )
        : key_{ key }
        , in_flight_requests_count_{ 0 }
        , candidates_{}
    { 
        for ( ; i != e; ++i )
            add_candidate( i->first, i->second );
    }

public:
    void
    flag_candidate_as_valid
        ( id const& candidate_id )
    { 
        -- in_flight_requests_count_; 

        auto i = find_candidate( candidate_id );
        assert( i != candidates_.end() 
              && i->second.state_ == candidate::STATE_CONTACTED
              && "candidate is already known");

        i->second.state_ = candidate::STATE_RESPONDED;
    }

    void
    flag_candidate_as_invalid
        ( id const& candidate_id )
    { 
        -- in_flight_requests_count_; 

        auto i = find_candidate( candidate_id );
        assert( i != candidates_.end() 
              && i->second.state_ == candidate::STATE_CONTACTED
              && "candidate is already known" );

        i->second.state_ = candidate::STATE_TIMEOUTED;
    }

    std::vector< candidate >
    select_new_closest_candidates
        ( std::size_t max_count )
    {
        std::vector< candidate > candidates;

        // Iterate over all candidates until we picked
        // candidates_max_count not-contacted candidates.
        for ( auto i = candidates_.begin(), e = candidates_.end()
            ; i != e && in_flight_requests_count_ < max_count 
            ; ++ i)
        {
            if ( i->second.state_ == candidate::STATE_UNKNOWN )
            {
                i->second.state_ = candidate::STATE_CONTACTED;
                ++ in_flight_requests_count_;
                candidates.push_back( i->second );
            }
        }

        return std::move( candidates );
    }

    /**
     *
     */
    std::vector< candidate >
    select_closest_valid_candidates
        ( std::size_t max_count )
    {
        std::vector< candidate > candidates;

        // Iterate over all candidates until we picked
        // candidates_max_count not-contacted candidates.
        for ( auto i = candidates_.begin(), e = candidates_.end()
            ; i != e && candidates.size() < max_count 
            ; ++ i)
        {
            if ( i->second.state_ == candidate::STATE_RESPONDED )
                candidates.push_back( i->second );
        }

        return std::move( candidates );
    }

    /**
     *
     */
    template< typename Candidates >
    bool
    are_these_candidates_closest
        ( Candidates const& candidates )
    {
        // Keep track of the closest candidate before
        // new candidate insertion.
        auto closest_candidate = candidates_.begin();

        for ( auto const& new_candidate : candidates )
            add_candidate( new_candidate.id_, new_candidate.endpoint_ );

        // If no closest candidate has been found.
        if ( closest_candidate == candidates_.begin() )
            return false;

        return true;
    }

    bool
    have_all_requests_completed
        ( void )
        const
    { return in_flight_requests_count_ == 0; }
    
    id const&
    get_key
        ( void )
        const
    { return key_; }

private:
    ///
    using candidates_type = std::map< id, candidate >;

private:
    void
    add_candidate
        ( id const& candidate_id
        , message_socket::endpoint_type const& e )
    {
        auto const distance = detail::distance( candidate_id, key_ );
        candidate const c{ candidate_id, e, candidate::STATE_UNKNOWN };
        candidates_.emplace( distance, c );
    }

    candidates_type::iterator
    find_candidate
        ( id const& candidate_id )
    {
        auto const distance = detail::distance( candidate_id, key_ );
        return candidates_.find( distance );
    }

private:
    id key_;
    std::size_t in_flight_requests_count_;
    candidates_type candidates_;
};

} // namespace detail
} // namespace kademlia

#endif

