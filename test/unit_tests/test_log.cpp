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

#include <iostream>
#include <boost/test/tools/output_test_stream.hpp>

#include "kademlia/log.hpp"

namespace {

namespace kd = kademlia::detail;

struct rdbuf_saver
{
    rdbuf_saver
        ( std::ostream & stream
        , std::streambuf * buffer )
            : stream_( stream )
            , old_buffer_( stream.rdbuf( buffer ) )
    { }

    ~rdbuf_saver
        ( void )
    { stream_.rdbuf( old_buffer_ ); }

    std::ostream & stream_;
    std::streambuf * old_buffer_;
};

BOOST_AUTO_TEST_SUITE( log )

BOOST_AUTO_TEST_SUITE( test_usage )

BOOST_AUTO_TEST_CASE( can_write_to_debug_log )
{
    boost::test_tools::output_test_stream out;

    {
        rdbuf_saver const s{ std::cout, out.rdbuf() };
        auto const ptr = reinterpret_cast< void *>( 0x12345678 );
        kd::get_debug_log( "test", ptr ) << "message" << std::endl;
    }

    BOOST_REQUIRE( out.is_equal( "[debug] (test @ 345678) message\n" ) );
}

BOOST_AUTO_TEST_CASE( can_write_to_debug_log_using_macro )
{
    boost::test_tools::output_test_stream out;

    {
        rdbuf_saver const s{ std::cout, out.rdbuf() };
        auto const ptr = reinterpret_cast< void *>( 0x12345678 );
        LOG_DEBUG( test, ptr ) << "message" << std::endl;
    }

#ifdef KADEMLIA_ENABLE_DEBUG
    BOOST_REQUIRE( out.is_equal( "[debug] (test @ 345678) message\n" ) );
#else
    BOOST_REQUIRE( out.is_equal( "" ) );
#endif
}

BOOST_AUTO_TEST_CASE( can_enable_log_module )
{
    // By default, unit tests enable log on all modules.
    kd::disable_log_for( "*" );

    BOOST_REQUIRE( ! kd::is_log_enabled( "test1" ) );
    BOOST_REQUIRE( ! kd::is_log_enabled( "test2" ) );

    kd::enable_log_for( "test1" );
    BOOST_REQUIRE( kd::is_log_enabled( "test1" ) );
    BOOST_REQUIRE( ! kd::is_log_enabled( "test2" ) );

    kd::enable_log_for( "*" );
    BOOST_REQUIRE( kd::is_log_enabled( "test1" ) );
    BOOST_REQUIRE( kd::is_log_enabled( "test2" ) );

}

BOOST_AUTO_TEST_CASE( can_convert_container_to_string )
{
    {
        std::vector< std::uint8_t > const c{ 'a', 'b', 'c' };
        auto const r = kd::to_string( c );

        BOOST_REQUIRE_EQUAL( "abc", r );
    }
    {
        std::vector< std::uint8_t > const c{ 1, 2, 3 };
        auto const r = kd::to_string( c );

        BOOST_REQUIRE_EQUAL( "\\1\\2\\3", r );
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}

