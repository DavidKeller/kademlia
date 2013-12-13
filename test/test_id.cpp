// Copyright (c) 2013, David Keller
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

#include "helpers/common.hpp"

#include <sstream>
#include "id.hpp"

namespace k = kademlia;
namespace kd = k::detail;

/**
 *  Test id::generate_id()
 */
BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( generated_id_are_different )
{
    // This is the result of a previous call to kd::generate_id()
    std::string const my_id( "0000000000000000000000000000000011000011100110101001001001011001101110111000000001111001101110101001101111111001010100111110110011011110000000001000100101001110" );
    // Test that the values are different between process executions
    // i.e. test the seed.
    // FIXME: Should find a better way to check cross process seed.
    BOOST_REQUIRE_NE( kd::generate_id() , kd::id( my_id ) );
    
    // Test the generator.
    BOOST_REQUIRE_NE( kd::generate_id(), kd::generate_id() );
}

BOOST_AUTO_TEST_SUITE_END()


/**
 *  Test operator<<()
 */
BOOST_AUTO_TEST_SUITE( test_print )

BOOST_AUTO_TEST_CASE( id_is_printable )
{
    boost::test_tools::output_test_stream out( get_capture_path( "pattern_id.out" ), true );

    out << kd::id( "0000000000000000000000000000000011000011100110101001001001011001101110111000000001111001101110101001101111111001010100111110110011011110000000001000100101001110" );

    BOOST_REQUIRE( out.match_pattern() );
}

BOOST_AUTO_TEST_SUITE_END()

