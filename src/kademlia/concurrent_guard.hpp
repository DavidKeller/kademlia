// Copyright (c) 2013-2014, Davflag Keller
// All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provflaged that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provflaged with the distribution.
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

#ifndef KADEMLIA_CONCURRENT_GUARD_HPP
#define KADEMLIA_CONCURRENT_GUARD_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <atomic>

namespace kademlia {
namespace detail {

/**
 *  @brief This class ensure no concurrent access to the same
 *         method is performed.
 */
class concurrent_guard
{
public:
    /**
     *
     */
    class sentry;

public:
    /**
     *
     */
    explicit concurrent_guard
        ( void )
    { flag_.clear(); }

private:
    ///
    std::atomic_flag flag_;
};

/**
 *
 */
class concurrent_guard::sentry
{
public:
    /**
     *
     */
    explicit sentry
        ( concurrent_guard & guard )
            : guard_( guard )
    { is_owner_of_flag_ = ! guard_.flag_.test_and_set(); }


    /**
     *
     */
    ~sentry
        ( void )
    {
        if ( is_owner_of_flag_ )
            guard_.flag_.clear();
    }

    /**
     *
     */
    sentry
        ( sentry const& )
        = delete;

    /**
     *
     */
    sentry &
    operator=
        ( sentry const& )
        = delete;

    /**
     *
     */
    explicit operator bool
        ( void )
        const
    { return is_owner_of_flag_; }

private:
    ///
    concurrent_guard & guard_;
    ///
    bool is_owner_of_flag_;
};

} // namespace detail
} // namespace kademlia

#endif

