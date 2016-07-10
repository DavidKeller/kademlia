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

#ifndef KADEMLIA_LOOKUP_TASK_HPP
#define KADEMLIA_LOOKUP_TASK_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <cassert>
#include <map>
#include <vector>

#include "kademlia/peer.hpp"
#include "kademlia/log.hpp"

namespace kademlia {
namespace detail {

///
class lookup_task
{
public:
    /**
     *
     */
    void
    flag_candidate_as_valid
        ( id const& candidate_id );

    /**
     *
     */
    void
    flag_candidate_as_invalid
        ( id const& candidate_id );

    /**
     *
     */
    std::vector< peer >
    select_new_closest_candidates
        ( std::size_t max_count );

    /**
     *
     */
    std::vector< peer >
    select_closest_valid_candidates
        ( std::size_t max_count );

    /**
     *
     */
    template< typename Peers >
    void
    add_candidates 
        ( Peers const& peers );

    /**
     *
     */
    bool
    have_all_requests_completed
        ( void )
        const;

    /**
     *
     */
    id const&
    get_key
        ( void )
        const;

protected:
    /**
     *
     */
    ~lookup_task
        ( void );

    /**
     *
     */
    template< typename Iterator >
    lookup_task
        ( id const & key
        , Iterator i, Iterator e );

private:
    ///
    struct candidate final
    {
        peer peer_;
        enum {
            STATE_UNKNOWN,
            STATE_CONTACTED,
            STATE_RESPONDED,
            STATE_TIMEOUTED,
        } state_;
    };

    ///
    using candidates_type = std::map< id, candidate >;

private:
    /**
     *
     */
    void
    add_candidate
        ( peer const& p );

    /**
     *
     */
    candidates_type::iterator
    find_candidate
        ( id const& candidate_id );

private:
    ///
    id key_;
    ///
    std::size_t in_flight_requests_count_;
    ///
    candidates_type candidates_;
};

inline
lookup_task::~lookup_task
    ( void )
    = default;

template< typename Iterator >
inline
lookup_task::lookup_task
    ( id const & key
    , Iterator i, Iterator e )
        : key_{ key }
        , in_flight_requests_count_{ 0 }
        , candidates_{}
{
    for ( ; i != e; ++i )
        add_candidate( peer{ i->first, i->second } );
}

inline void
lookup_task::flag_candidate_as_valid
    ( id const& candidate_id )
{
    auto i = find_candidate( candidate_id );
    if ( i == candidates_.end() )
        return;

    -- in_flight_requests_count_;
    i->second.state_ = candidate::STATE_RESPONDED;
}

inline void
lookup_task::flag_candidate_as_invalid
    ( id const& candidate_id )
{
    auto i = find_candidate( candidate_id );
    if ( i == candidates_.end() )
        return;

    -- in_flight_requests_count_;
    i->second.state_ = candidate::STATE_TIMEOUTED;
}

inline std::vector< peer >
lookup_task::select_new_closest_candidates
    ( std::size_t max_count )
{
    std::vector< peer > candidates;

    // Iterate over all candidates until we picked
    // candidates_max_count not-contacted candidates.
    for ( auto i = candidates_.begin(), e = candidates_.end()
        ; i != e && in_flight_requests_count_ < max_count
        ; ++ i )
    {
        if ( i->second.state_ == candidate::STATE_UNKNOWN )
        {
            i->second.state_ = candidate::STATE_CONTACTED;
            ++ in_flight_requests_count_;
            candidates.push_back( i->second.peer_ );
        }
    }

    return candidates;
}

inline std::vector< peer >
lookup_task::select_closest_valid_candidates
    ( std::size_t max_count )
{
    std::vector< peer > candidates;

    // Iterate over all candidates until we picked
    // candidates_max_count responsive candidates.
    for ( auto i = candidates_.begin(), e = candidates_.end()
        ; i != e && candidates.size() < max_count
        ; ++ i )
    {
        if ( i->second.state_ == candidate::STATE_RESPONDED )
            candidates.push_back( i->second.peer_ );
    }

    return candidates;
}

template< typename Peers >
inline void
lookup_task::add_candidates
    ( Peers const& peers )
{
    for ( auto const& p : peers )
        add_candidate( p );
}

inline bool
lookup_task::have_all_requests_completed
    ( void )
    const
{ return in_flight_requests_count_ == 0; }

inline id const&
lookup_task::get_key
    ( void )
    const
{ return key_; }

inline void
lookup_task::add_candidate
    ( peer const& p )
{
    LOG_DEBUG( lookup_task, this )
            << "adding '" << p << "'." << std::endl;

    auto const d = distance( p.id_, key_ );
    candidate const c{ p, candidate::STATE_UNKNOWN };
    candidates_.emplace( d, c );
}

inline lookup_task::candidates_type::iterator
lookup_task::find_candidate
    ( id const& candidate_id )
{
    auto const d = distance( candidate_id, key_ );
    return candidates_.find( d );
}

} // namespace detail
} // namespace kademlia

#endif

