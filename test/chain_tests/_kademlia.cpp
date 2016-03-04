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

#include <boost/python.hpp>

#include "test_engine.hpp"

namespace {

namespace k = kademlia;
namespace kt = k::test;
namespace kd = k::detail;
namespace p = boost::python;

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

        .def( repr( p::self ) );

    p::enum_< kd::header::type >( "MessageType" )
        .value( "PING_REQUEST", kd::header::PING_REQUEST )
        .value( "PING_RESPONSE", kd::header::PING_RESPONSE )
        .value( "STORE_REQUEST", kd::header::STORE_REQUEST )
        .value( "FIND_PEER_REQUEST", kd::header::FIND_PEER_REQUEST )
        .value( "FIND_PEER_RESPONSE", kd::header::FIND_PEER_RESPONSE )
        .value( "FIND_VALUE_REQUEST", kd::header::FIND_VALUE_REQUEST )
        .value( "FIND_VALUE_RESPONSE", kd::header::FIND_VALUE_RESPONSE );

    p::class_< kt::packet >
        ( "Message"
        , p::init< k::endpoint const&
                 , k::endpoint const&
                 , kd::header::type const& >() )
                     
        .def( "from_endpoint"
            , &kt::packet::from
            , p::return_value_policy< p::copy_const_reference >() )

        .def( "to_endpoint"
            , &kt::packet::to
            , p::return_value_policy< p::copy_const_reference >() )

        .def( "type"
            , &kt::packet::type
            , p::return_value_policy< p::copy_const_reference >() );

    p::def( "count_messages", &kt::count_packets );

    p::def( "pop_message", &kt::pop_packet );

    p::def( "clear_messages", &kt::clear_packets );

    p::def( "forget_attributed_ip", &kt::forget_attributed_ip );

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

        .def( repr( p::self ) );

    p::class_< kt::test_engine, boost::noncopyable >
        ( "FirstSession"
        , p::init< boost::asio::io_service &
                 , k::endpoint const& 
                 , k::endpoint const&
                 , kd::id const& >()
        [ p::with_custodian_and_ward< 1, 2 >() ] )

        .def( "ipv4"
            , &kt::test_engine::ipv4 )
            
        .def( "ipv6"
            , &kt::test_engine::ipv6 );

    p::class_< kt::test_engine, boost::noncopyable >
        ( "Session"
        , p::init< boost::asio::io_service &
                 , k::endpoint const& 
                 , k::endpoint const& 
                 , k::endpoint const&
                 , kd::id const& >()
        [ p::with_custodian_and_ward< 1, 2 >() ] )

        .def( "ipv4"
            , &kt::test_engine::ipv4 )
            
        .def( "ipv6"
            , &kt::test_engine::ipv6 )

        .def( "async_save"
            , &kt::test_engine::async_save< p::object > )

        .def( "async_load"
            , &kt::test_engine::async_load< p::object > );
}

