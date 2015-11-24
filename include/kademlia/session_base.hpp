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

#ifndef KADEMLIA_SESSION_BASE_HPP
#define KADEMLIA_SESSION_BASE_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <cstdint>
#include <vector>
#include <system_error>
#include <functional>

#include <kademlia/detail/cxx11_macros.hpp>

namespace kademlia {

/**
 *  @brief This object contains session types.
 */
class session_base
{
public:
    /// The key type used to find data.
    using key_type = std::vector< std::uint8_t >;

    /// The stored data type.
    using data_type = std::vector< std::uint8_t >;

    /// The callback type called to signal an async save status.
    using save_handler_type = std::function
            < void
                ( std::error_code const& error )
            >;
    /// The callback type called to signal an async load status.
    using load_handler_type = std::function
            < void
                ( std::error_code const& error
                , data_type const& data )
            >;

    /// This kademlia implementation default port.
    static CXX11_CONSTEXPR std::uint16_t DEFAULT_PORT = 27980;

protected:
    /**
     *  @brief Destructor used to prevent
     *         usage dervied classes as this
     *         base.
     */
    ~session_base
        ( void )
        = default;
};

} // namespace kademlia

#endif

