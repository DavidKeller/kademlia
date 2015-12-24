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

#include <boost/python.hpp>

#include "utils/fake_socket.hpp"

#include <kademlia/session_base.hpp>
#include <kademlia/endpoint.hpp>

#include "kademlia/log.hpp"
#include "kademlia/buffer.hpp"
#include "kademlia/engine.hpp"

namespace k = kademlia;
namespace kd = k::detail;
namespace p = boost::python;

namespace {

using test_engine = kd::engine< k::test::fake_socket >;

struct engine
{
    engine
        ( boost::asio::io_service & service
        , k::endpoint const & ipv4
        , k::endpoint const & ipv6
        , kd::id const& new_id )
            : io_service_( service )
            , work_( service )
            , engine_( service
                     , ipv4, ipv6, new_id )
            , listen_ipv4_( k::test::fake_socket::get_last_allocated_ipv4()
                          , k::session_base::DEFAULT_PORT )
            , listen_ipv6_( k::test::fake_socket::get_last_allocated_ipv6()
                          , k::session_base::DEFAULT_PORT )
    { }

    engine
        ( boost::asio::io_service & service
        , k::endpoint const & initial_peer
        , k::endpoint const & ipv4
        , k::endpoint const & ipv6
        , kd::id const& new_id )
            : io_service_( service )
            , work_( service )
            , engine_( service
                     , initial_peer
                     , ipv4, ipv6
                     , new_id )
            , listen_ipv4_( k::test::fake_socket::get_last_allocated_ipv4()
                          , k::session_base::DEFAULT_PORT )
            , listen_ipv6_( k::test::fake_socket::get_last_allocated_ipv6()
                          , k::session_base::DEFAULT_PORT )
    { }

    void
    async_save
        ( std::string const& key
        , std::string const& data
        , p::object & callable )
    {
        test_engine::key_type const k{ key.begin(), key.end() };
        test_engine::data_type const d{ data.begin(), data.end() };
        engine_.async_save( k, d, callable );
    }

    void
    async_load
        ( std::string const& key
        , p::object & callable )
    {
        test_engine::key_type const k{ key.begin(), key.end() };
        auto c = [ callable ]( std::error_code const& failure
                             , test_engine::data_type const& data )
        {
            callable( failure, std::string{ data.begin(), data.end() } );
        };

        engine_.async_load( k, c );
    }

    k::endpoint
    ipv4
        ( void )
        const
    {
        return k::endpoint( listen_ipv4_.address().to_string()
                          , listen_ipv4_.port() );
    }

    k::endpoint
    ipv6
        ( void )
        const
    {
        return k::endpoint( listen_ipv6_.address().to_string()
                          , listen_ipv6_.port() );
    }

    boost::asio::io_service & io_service_;
    boost::asio::io_service::work work_;
    test_engine engine_;
    k::test::fake_socket::endpoint_type listen_ipv4_;
    k::test::fake_socket::endpoint_type listen_ipv6_;
};

struct message final
{
    message
        ( k::endpoint const& from
        , k::endpoint const& to
        , kd::header::type const& type )
            : from_( from ), to_( to ), type_( type )
    { }

    bool
    operator==
        ( message const& m )
    { return from_ == m.from_ && to_ == m.to_ && type_ == m.type_; }

    bool
    operator!=
        ( message const& m )
    { return ! operator==( m ); }

    k::endpoint from_;
    k::endpoint to_;
    kd::header::type type_;
};

std::ostream &
operator<<
    ( std::ostream & out, message const& m )
{ return out << m.from_ << " -> " << m.to_ << " = " << m.type_; }

kd::header
extract_header
    ( k::test::fake_socket::packet const& p )
{
    kd::header h;

    auto i = p.data_.begin(), e = p.data_.end();
    deserialize( i, e, h );

    return h;
}

message
pop_message
    ( void )
{
    auto & packets = k::test::fake_socket::get_packets();

    if ( packets.empty() )
        throw std::runtime_error{ "no message left" };

    auto const& p = packets.front();

    message const m{ k::endpoint{ p.from_.address().to_string()
                                , std::to_string( p.from_.port() ) }
                   , k::endpoint{ p.to_.address().to_string()
                                , std::to_string( p.to_.port() ) }
                   , extract_header( p ).type_ };
    packets.pop(); 

    return m;
}

std::size_t
count_messages
    ( void )
{
    return k::test::fake_socket::get_packets().size();
}

void
clear_messages
    ( void )
{ 
    auto & packets = k::test::fake_socket::get_packets();

    while ( ! packets.empty() )
        packets.pop();
}

void
forget_attributed_ip
    ( void )
{
    using k::test::fake_socket;
    fake_socket::get_last_allocated_ipv4() = fake_socket::get_first_ipv4();
    fake_socket::get_last_allocated_ipv6() = fake_socket::get_first_ipv6();
}

std::size_t ( boost::asio::io_service::*io_service_poll )( void )
    = &boost::asio::io_service::poll;

} // anonymous namespace

BOOST_PYTHON_MODULE( _kademlia )
{
    p::def( "enable_log_for", &kd::enable_log_for );

    p::scope().attr( "DEFAULT_PORT" ) = k::session_base::DEFAULT_PORT;

    p::class_< boost::asio::io_service, boost::noncopyable >
        ( "Service" )
        
        .def( "poll"
            , io_service_poll );

    p::class_< boost::asio::io_service::work, boost::noncopyable >
        ( "ServiceWork"
        , p::init< boost::asio::io_service & >()
        [ p::with_custodian_and_ward< 1, 2 >() ] );

    p::class_< k::endpoint >
        ( "Endpoint"
        , p::init< std::string const&
                 , std::string const& >() )

        .def( p::init< std::string const&
                     , std::uint16_t >() )
                     
        .def( str( p::self ) );

    p::enum_< kd::header::type >( "MessageType" )
        .value( "PING_REQUEST", kd::header::PING_REQUEST )
        .value( "PING_RESPONSE", kd::header::PING_RESPONSE )
        .value( "STORE_REQUEST", kd::header::STORE_REQUEST )
        .value( "FIND_PEER_REQUEST", kd::header::FIND_PEER_REQUEST )
        .value( "FIND_PEER_RESPONSE", kd::header::FIND_PEER_RESPONSE )
        .value( "FIND_VALUE_REQUEST", kd::header::FIND_VALUE_REQUEST )
        .value( "FIND_VALUE_RESPONSE", kd::header::FIND_VALUE_RESPONSE );

    p::class_< message >
        ( "Message"
        , p::init< k::endpoint const&
                 , k::endpoint const&
                 , kd::header::type const& >() )
                     
        .def( "__eq__"
            , &message::operator == )

        .def( "__ne__"
            , &message::operator != )

        .def( str( p::self ) );

    p::def( "count_messages", &count_messages );

    p::def( "pop_message", &pop_message );

    p::def( "clear_messages", &clear_messages );

    p::def( "forget_attributed_ip", &forget_attributed_ip );

    p::class_< std::error_code >
        ( "Error"
        , p::no_init )

        .def( "__str__"
            , &std::error_code::message )

        .def( "__nonzero__"
            , &std::error_code::operator bool );

    p::class_< kd::id >
        ( "Id"
        , p::init<>() )

        .def( p::init< std::string >() )

        .def( str( p::self ) );

    p::class_< engine, boost::noncopyable >
        ( "FirstSession"
        , p::init< boost::asio::io_service &
                 , k::endpoint const& 
                 , k::endpoint const&
                 , kd::id const& >()
        [ p::with_custodian_and_ward< 1, 2 >() ] )

        .def( "ipv4"
            , &engine::ipv4 )
            
        .def( "ipv6"
            , &engine::ipv6 );

    p::class_< engine, boost::noncopyable >
        ( "Session"
        , p::init< boost::asio::io_service &
                 , k::endpoint const& 
                 , k::endpoint const& 
                 , k::endpoint const&
                 , kd::id const& >()
        [ p::with_custodian_and_ward< 1, 2 >() ] )

        .def( "ipv4"
            , &engine::ipv4 )
            
        .def( "ipv6"
            , &engine::ipv6 )

        .def( "async_save"
            , &engine::async_save )

        .def( "async_load"
            , &engine::async_load );
}

