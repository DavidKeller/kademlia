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

#include "message.hpp"

#include "helpers/common.hpp"

#include <random>

namespace k = kademlia;
namespace kd = k::detail;

/**
 */
BOOST_AUTO_TEST_SUITE( test_message )

BOOST_AUTO_TEST_CASE( can_serialize_message  )
{
    std::default_random_engine random_engine;

    kd::header const header_to_serialize =
        { kd::header::V1
        , kd::header::FIND_VALUE_RESPONSE
        , kd::id{ random_engine }
        , kd::id{ random_engine } };

    kd::buffer buffer;
    kd::serialize( header_to_serialize, buffer );

    kd::header deserialized_header;
    auto i = buffer.cbegin(), e = buffer.cend();
    BOOST_REQUIRE( ! kd::deserialize( i, e, deserialized_header ) );
    BOOST_REQUIRE( i == e );

    BOOST_REQUIRE_EQUAL( header_to_serialize.version_, deserialized_header.version_ );
    BOOST_REQUIRE_EQUAL( header_to_serialize.type_, deserialized_header.type_);

    BOOST_REQUIRE_EQUAL_COLLECTIONS( header_to_serialize.source_id_.begin_block()
                                   , header_to_serialize.source_id_.end_block()
                                   , deserialized_header.source_id_.begin_block()
                                   , deserialized_header.source_id_.end_block() );

    BOOST_REQUIRE_EQUAL_COLLECTIONS( header_to_serialize.random_token_.begin_block()
                                   , header_to_serialize.random_token_.end_block()
                                   , deserialized_header.random_token_.begin_block()
                                   , deserialized_header.random_token_.end_block() );
}
        
BOOST_AUTO_TEST_SUITE_END()

