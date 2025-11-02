// Copyright (c) 2014, David Keller
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

#include "log.hpp"

#include <iostream>
#include <set>

namespace kademlia {
namespace detail {

namespace {

using enabled_modules_log_type = std::set< std::string >;

enabled_modules_log_type &
get_enabled_modules
    ( void )
{
    static enabled_modules_log_type enabled_modules_;
    return enabled_modules_;
}

} // anonymous namespace

std::ostream &
get_debug_log
    ( char const * module
    , void const * thiz )
{
    return std::cout << "[debug] (" << module << " @ "
                     << std::hex << ( std::uintptr_t( thiz ) & 0xffffff )
                     << std::dec << ") ";
}

/**
 *
 */
void
enable_log_for
    ( std::string const& module )
{ get_enabled_modules().insert( module ); }

/**
 *
 */
void
disable_log_for
    ( std::string const& module )
{ get_enabled_modules().erase( module ); }

/**
 *
 */
bool
is_log_enabled
    ( std::string const& module )
{
    return get_enabled_modules().count( "*" ) > 0
            || get_enabled_modules().count( module ) > 0;
}

} // namespace detail
} // namespace kademlia

