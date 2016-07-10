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

#include <memory>

#include <boost/asio/io_service.hpp>

#include <kademlia/session_base.hpp>
#include <kademlia/endpoint.hpp>

#include "kademlia/log.hpp"
#include "kademlia/buffer.hpp"
#include "kademlia/engine.hpp"

#include "fake_socket.hpp"

namespace kademlia {
namespace test {

class test_engine final
{
public:
    test_engine
        ( boost::asio::io_service & service
        , endpoint const & ipv4
        , endpoint const & ipv6
        , detail::id const& new_id )
            : work_( service )
            , engine_( service
                     , ipv4, ipv6, new_id )
            , listen_ipv4_( fake_socket::get_last_allocated_ipv4()
                          , session_base::DEFAULT_PORT )
            , listen_ipv6_( fake_socket::get_last_allocated_ipv6()
                          , session_base::DEFAULT_PORT )
    { }

    test_engine
        ( boost::asio::io_service & service
        , endpoint const & initial_peer
        , endpoint const & ipv4
        , endpoint const & ipv6
        , detail::id const& new_id )
            : work_( service )
            , engine_( service
                     , initial_peer
                     , ipv4, ipv6
                     , new_id )
            , listen_ipv4_( fake_socket::get_last_allocated_ipv4()
                          , session_base::DEFAULT_PORT )
            , listen_ipv6_( fake_socket::get_last_allocated_ipv6()
                          , session_base::DEFAULT_PORT )
    { }

    template< typename Callable >
    void
    async_save
        ( std::string const& key
        , std::string const& data
        , Callable & callable )
    {
        impl::key_type const k{ key.begin(), key.end() };
        impl::data_type const d{ data.begin(), data.end() };
        engine_.async_save( k, d, callable );
    }

    template< typename Callable >
    void
    async_load
        ( std::string const& key
        , Callable & callable )
    {
        impl::key_type const k{ key.begin(), key.end() };
        auto c = [ callable ]( std::error_code const& failure
                             , impl::data_type const& data )
        {
            callable( failure, std::string{ data.begin(), data.end() } );
        };

        engine_.async_load( k, c );
    }

    endpoint
    ipv4
        ( void )
        const
    {
        return endpoint( listen_ipv4_.address().to_string()
                       , listen_ipv4_.port() );
    }

    endpoint
    ipv6
        ( void )
        const
    {
        return endpoint( listen_ipv6_.address().to_string()
                       , listen_ipv6_.port() );
    }

private:
    using impl = detail::engine< fake_socket >;

private:
    boost::asio::io_service::work work_;
    impl engine_;
    fake_socket::endpoint_type listen_ipv4_;
    fake_socket::endpoint_type listen_ipv6_;
};

class packet final
{
public:
    packet
        ( endpoint const& from
        , endpoint const& to
        , detail::header::type const& type )
            : from_( from ), to_( to ), type_( type )
    { }

    endpoint const&
    from
        ( void )
        const
    { return from_; }

    endpoint const&
    to
        ( void )
        const
    { return to_; }

    detail::header::type const&
    type
        ( void )
        const
    { return type_; }

private:
    endpoint from_;
    endpoint to_;
    detail::header::type type_;
};

inline detail::header
extract_kademlia_header
    ( fake_socket::packet const& p )
{
    detail::header h;

    auto i = p.data_.begin(), e = p.data_.end();
    deserialize( i, e, h );

    return h;
}

inline packet
pop_packet
    ( void )
{
    auto & packets = fake_socket::get_logged_packets();

    if ( packets.empty() )
        throw std::runtime_error{ "no packet left" };

    auto const& p = packets.front();

    packet const r{ endpoint{ p.from_.address().to_string()
                            , std::to_string( p.from_.port() ) }
                  , endpoint{ p.to_.address().to_string()
                            , std::to_string( p.to_.port() ) }
                  , extract_kademlia_header( p ).type_ };
    packets.pop(); 

    return r;
}

inline std::size_t
count_packets
    ( void )
{
    return fake_socket::get_logged_packets().size();
}

inline void
clear_packets
    ( void )
{ 
    while ( count_packets() > 0ULL )
        pop_packet();
}

inline void
forget_attributed_ip
    ( void )
{
    fake_socket::get_last_allocated_ipv4() = fake_socket::get_first_ipv4();
    fake_socket::get_last_allocated_ipv6() = fake_socket::get_first_ipv6();
}

} // namespace test
} // namespace kademlia

