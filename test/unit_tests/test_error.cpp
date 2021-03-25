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


#include "kademlia/error.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <cctype>

namespace {

namespace k = kademlia;

bool
compare_enum_to_message( char const * name, k::error_type const& error )
{
    auto message = make_error_condition( error ).message();

    std::replace( message.begin(), message.end(), ' ', '_' );
    std::transform( message.begin()
                  , message.end(), message.begin(),
                  ::toupper );

    return name == message;
}

TEST(ErrorTest, error_message_follows_the_error_name)
{
    EXPECT_TRUE(compare_enum_to_message("UNKNOWN_ERROR", k::UNKNOWN_ERROR));
    EXPECT_TRUE(compare_enum_to_message("RUN_ABORTED", k::RUN_ABORTED));
    EXPECT_TRUE(compare_enum_to_message("INITIAL_PEER_FAILED_TO_RESPOND", k::INITIAL_PEER_FAILED_TO_RESPOND));
    EXPECT_TRUE(compare_enum_to_message("INVALID_ID", k::INVALID_ID));
    EXPECT_TRUE(compare_enum_to_message("TRUNCATED_ID", k::TRUNCATED_ID));
    EXPECT_TRUE(compare_enum_to_message("TRUNCATED_HEADER", k::TRUNCATED_HEADER));
    EXPECT_TRUE(compare_enum_to_message("TRUNCATED_ENDPOINT", k::TRUNCATED_ENDPOINT));
    EXPECT_TRUE(compare_enum_to_message("TRUNCATED_ADDRESS", k::TRUNCATED_ADDRESS));
    EXPECT_TRUE(compare_enum_to_message("TRUNCATED_SIZE", k::TRUNCATED_SIZE));
    EXPECT_TRUE(compare_enum_to_message("UNKNOWN_PROTOCOL_VERSION", k::UNKNOWN_PROTOCOL_VERSION));
    EXPECT_TRUE(compare_enum_to_message("CORRUPTED_BODY", k::CORRUPTED_BODY));
    EXPECT_TRUE(compare_enum_to_message("UNASSOCIATED_MESSAGE_ID", k::UNASSOCIATED_MESSAGE_ID));
    EXPECT_TRUE(compare_enum_to_message("INVALID_IPV4_ADDRESS", k::INVALID_IPV4_ADDRESS));
    EXPECT_TRUE(compare_enum_to_message("INVALID_IPV6_ADDRESS", k::INVALID_IPV6_ADDRESS));
    EXPECT_TRUE(compare_enum_to_message("UNIMPLEMENTED", k::UNIMPLEMENTED));
    EXPECT_TRUE(compare_enum_to_message("VALUE_NOT_FOUND", k::VALUE_NOT_FOUND));
    EXPECT_TRUE(compare_enum_to_message("TIMER_MALFUNCTION", k::TIMER_MALFUNCTION));
    EXPECT_TRUE(compare_enum_to_message("ALREADY_RUNNING", k::ALREADY_RUNNING));
}

TEST(ErrorTest, error_category_is_kademlia)
{
    auto e = make_error_condition( k::UNKNOWN_ERROR );
    EXPECT_STREQ( "kademlia", e.category().name());
}

}

