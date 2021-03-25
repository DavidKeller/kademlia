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

#include "common.hpp"
#include "kademlia/id.hpp"
#include "gtest/gtest.h"
#include <system_error>

namespace {

namespace k = kademlia;
namespace kd = k::detail;


TEST(IDTest, random_generated_id_are_different)
{
    std::default_random_engine random_engine;

    EXPECT_NE(kd::id{ random_engine }
                    , kd::id{ random_engine });
}

TEST(IDTest, emptry_string_generated_id_is_valid)
{
    kd::id const new_id{ "" };

    auto equal_to_zero = [] (kd::id::block_type v)
    { return !v; };

    EXPECT_TRUE(std::all_of(new_id.begin()
                              , new_id.end()
                              , equal_to_zero));
}

TEST(IDTest, invalid_string_cannot_generate_id)
{
    EXPECT_THROW(kd::id{ "?" }, std::system_error);
    EXPECT_THROW(kd::id{ "1?2" }, std::system_error);
    EXPECT_THROW(kd::id{ "x" }, std::system_error);
    EXPECT_THROW(kd::id{ " " }, std::system_error);
    EXPECT_THROW(kd::id{ "                        "
                                 "                        "
                                 "                        " }
                       , std::system_error);
}

TEST(IDTest, valid_string_generates_valid_id)
{
    {
        kd::id const new_id{ "fedcba9876543210" };

        std::size_t i = 0;
        while (i != 96)
            EXPECT_FALSE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_TRUE(new_id[i++]);

        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
        EXPECT_FALSE(new_id[i++]);
    }
    {
        kd::id const new_id{ "8000"
                             "0000"
                             "0000"
                             "0000"
                             "0000"
                             "0000"
                             "0000"
                             "0000"
                             "0000"
                             "0001" };

        EXPECT_TRUE(new_id[0]);
        EXPECT_FALSE(new_id[1]);
        EXPECT_FALSE(new_id[158]);
        EXPECT_TRUE(new_id[159]);
    }
}

TEST(IDTest, hash_generated_id_are_valid)
{
    kd::id::value_to_hash_type value_to_hash(512);
    std::generate(value_to_hash.begin(), value_to_hash.end()
                 , &std::rand);

    kd::id const id{ value_to_hash };
    EXPECT_EQ(id, kd::id{ value_to_hash });

    std::generate(value_to_hash.begin(), value_to_hash.end()
                 , &std::rand);
    EXPECT_NE(id, kd::id{ value_to_hash });
}


TEST(IDTest, id_can_be_sorted)
{
    EXPECT_LT(kd::id{ "0" }, kd::id{ "1" });
    EXPECT_LT(kd::id{ "1" }, kd::id{ "2" });
    EXPECT_LT(kd::id{ "1" }, kd::id{ "3" });
    EXPECT_LT(kd::id{ "1" }, kd::id{ "8000000000000" });
}

TEST(IDTest, id_bit_can_be_updated)
{
    kd::id i{ "0" };
    i[0] = true;
    i[kd::id::BIT_SIZE - 1] = true;
    i[kd::id::BIT_SIZE - 2] = true;

    EXPECT_EQ(kd::id{ "8000000000000000000000000000000000000003" }, i);

    i[kd::id::BIT_SIZE - 1] = false;

    EXPECT_EQ(kd::id{ "8000000000000000000000000000000000000002" }, i);
}

TEST(IDTest, id_distance_can_be_evaluated)
{
    {
        std::default_random_engine random_engine;

        kd::id id{ random_engine };

        EXPECT_EQ(kd::id(), kd::distance(id, id));

    }
    {
        kd::id const id1{ "1" };
        kd::id const id2{ "2" };
        kd::id const id3{ "4" };

        EXPECT_LT(kd::distance(id1, id2)
                        , kd::distance(id1, id3));
    }
}

TEST(IDTest, id_is_printable)
{
    boost::test_tools::output_test_stream out(k::test::get_capture_path("pattern_id.out"), true);

    out << kd::id{ "0123456789abcdef" };

    EXPECT_TRUE(out.match_pattern());
}

}

