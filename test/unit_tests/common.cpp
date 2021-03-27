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

#include <cstdlib>

#include <iostream>

#include <boost/system/system_error.hpp>
#include <boost/filesystem.hpp>

#include "kademlia/log.hpp"

namespace filesystem = boost::filesystem;
namespace unit_test = boost::unit_test;

namespace kademlia {
namespace test {

namespace {

filesystem::path const tests_directory_{ TESTS_DIR };

}

std::string get_capture_path( std::string const & capture_name )
{
    return ( tests_directory_ / "captures" / capture_name ).string();
}

} // namespace test
} // namespace kademlia

/**
 *  When we are using boost as a shared (i.e. BOOST_TEST_DYN_LINK
 *  macro is defined), BOOST_TEST_ALTERNATIVE_INIT_API macro is
 *  automatically defined by boost unit-test config header.
 *  Hence don't bother searching this macro definition in
 *  this project build or source files.
 *
 *  That means with shared version of unit-test library, unit_test_main
 *  accepts a bool (*)() init function while it uses a
 *  test_suite * (*)(int, char *[]) when compiled as static.
 *
 *  See http://www.boost.org/doc/libs/1_55_0/libs/test/doc/html/utf/user-guide/test-runners.html
 */
#ifdef BOOST_TEST_ALTERNATIVE_INIT_API

bool
init_unit_test
    ( void )
{ 
    kademlia::detail::enable_log_for( "*" );
    return true;
}

#else

boost::unit_test::test_suite *
init_unit_test_suite
    ( int
    , char* [] )
{
    kademlia::detail::enable_log_for( "*" );
    return nullptr;
}

#endif

/**
 *  When using shared version of boost unit-test library, main
 *  function is not defined, hence provide it.
 *
 *  We can assume that BOOST_TEST_ALTERNATIVE_INIT_API is defined
 *  as the documentation says so when using shared library.
 *  See http://www.boost.org/doc/libs/1_55_0/libs/test/doc/html/utf/user-guide/test-runners.html
 */
#ifdef BOOST_TEST_DYN_LINK

int
main
    ( int argc
    , char * argv[] )
{ return boost::unit_test::unit_test_main( &init_unit_test, argc, argv ); }

#endif

