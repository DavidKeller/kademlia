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

#include "helpers/common.hpp"

#include <vector>
#include <utility>

#include "kademlia/id.hpp"
#include "kademlia/ip_endpoint.hpp"

#include "kademlia/store_value_task.hpp"

namespace k = kademlia;
namespace kd = k::detail;

#if 0
namespace {

using save_handler_type = std::function< void ( std::error_code const& ) >;

using data_type = std::vector< std::uint8_t >;

using routing_table_peer = std::pair< kd::id
                                    , kd::ip_endpoint >;

using task = kd::store_value_task< save_handler_type, data_type >;

} // anonymous namespace
#endif

/**
 */
BOOST_AUTO_TEST_SUITE( test_construction )

BOOST_AUTO_TEST_CASE( can_be_constructed_without_candidates )
{
#if 0
    std::vector< routing_table_peer > candidates;
    kd::id const key{};
    data_type const data{ 1, 2, 3, 4 };
    save_handler_type handler;

    task c{ key, data, candidates.begin(), candidates.end(), handler };

    BOOST_REQUIRE( data == c.get_data() );
#endif
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( test_usage )

BOOST_AUTO_TEST_CASE( can_notify_error_to_caller )
{
#if 0
    std::vector< routing_table_peer > candidates;
    kd::id const key{};
    data_type const data{ 1, 2, 3, 4 };

    std::error_code last_failure;
    auto handler = [ &last_failure ] ( std::error_code const& failure )
    { last_failure = failure; };

    task c{ key, data, candidates.begin(), candidates.end(), handler };

    BOOST_REQUIRE( std::error_code{} == last_failure );
    c.notify_caller( std::make_error_code( std::errc::invalid_argument ) );
    BOOST_REQUIRE( std::errc::invalid_argument == last_failure );
#endif
}

BOOST_AUTO_TEST_SUITE_END()

