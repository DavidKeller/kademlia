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

#ifndef KADEMLIA_SESSION_HPP
#define KADEMLIA_SESSION_HPP

#ifdef _MSC_VER
#   pragma once
#endif

#include <functional>
#include <cstdlib>
#include <deque>
#include <vector>
#include <cstdint>

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>

#include <kademlia/error.hpp>

namespace kademlia {

/**
 *
 */
class fake_socket
{
public:
    ///
    using endpoint_type = boost::asio::ip::udp::endpoint;

    ///
    using protocol_type = boost::asio::ip::udp;

public:
    /**
     *
     */
    fake_socket
        ( boost::asio::io_service & io_service
        , boost::asio::ip::udp const & protocol )
        : io_service_( io_service )
        , local_endpoint_( generate_unique_endpoint() )
        , pending_reads_()
    { add_route_to_socket( local_endpoint_, this ); }

    /**
     *
     */
    fake_socket
        ( fake_socket const& o )
        = delete;

    /**
     *
     */
    fake_socket
        ( fake_socket && o )
        : io_service_( o.io_service_ )
        , local_endpoint_( o.local_endpoint_ )
        , pending_reads_( std::move( o.pending_reads_ ) )
    { add_route_to_socket( local_endpoint_, this ); }

    /**
     *
     */
    fake_socket &
    operator=
        ( fake_socket const& o )
        = delete;

    /**
     *
     */
    ~fake_socket
        ( void )
    {
        // This socket does'nt must no longer
        // read messages.
        if ( get_socket( local_endpoint_ ) == this )
            add_route_to_socket( local_endpoint_, nullptr );
    }

    /**
     *
     */
    template< typename Option >
    void 
    set_option
        ( Option const& )
    { }

    /**
     *
     */
    boost::system::error_code
    bind
        ( endpoint_type const& )
    { return boost::system::error_code{}; }

    /**
     *
     */
    boost::system::error_code
    close
        ( boost::system::error_code & failure )
    { return failure = boost::system::error_code{}; }

    /**
     *
     */
    template< typename Callback >
    void
    async_receive_from
        ( boost::asio::mutable_buffer const& buffer
        , endpoint_type & from
        , Callback callback )
    { 
        // Check if there is packets waiting.
        if ( pending_writes_.empty() )
            // No packet are waiting, hence register that
            // the current socket is waiting for packet.
            pending_reads_.push_back( { buffer, from, std::move( callback ) } ); 
        else 
            // A packet is waiting to be read, so read it
            // asynchronously.
            async_execute_read( buffer, from, callback );
    }

    /**
     *
     */
    template< typename Callback >
    void
    async_send_to
        ( boost::asio::const_buffer const& buffer
        , endpoint_type const& to
        , Callback callback )
    { 
        // Ensure the destination socket is listening.
        auto target = get_socket( to );
        if ( ! target )
            callback( make_error_code( boost::system::errc::network_unreachable )
                    , 0ULL );

        // Check if it's not waiting for any packet.
        if ( target->pending_reads_.empty() )
           target->pending_writes_.push_back( { buffer, local_endpoint_
                                           , std::move( callback ) } );
        else
            // It's already waiting for the current packet.
            async_execute_write( target, buffer, callback );
    }

private:
    ///
    using callback_type = std::function
            < void ( boost::system::error_code const&
                   , std::size_t ) >;

    ///
    struct pending_read
    {
        boost::asio::mutable_buffer buffer_;
        endpoint_type & source_;
        callback_type callback_;
    };

    ///
    struct pending_write
    {
        boost::asio::const_buffer buffer_;
        endpoint_type const& source_;
        callback_type callback_;
    };

    ///
    using router = std::vector< fake_socket * >;

private:
    /**
     *  @note This function is not thread safe.
     */
    static endpoint_type
    generate_unique_endpoint
        ( void )
    {
        static std::uint32_t last_allocated_ipv4_ = 1UL;

        assert( last_allocated_ipv4_ != 0UL 
              && "allocated more than 2^32 ipv4" );

        boost::asio::ip::address_v4 const ipv4( last_allocated_ipv4_ );
        return endpoint_type( ipv4, 27980 );
    }

    /**
     *
     */
    static router &
    get_router
        ( void )
    {
        static router router_;
        return router_;
    }

    /**
     *
     */
    static void
    add_route_to_socket
        ( endpoint_type const& e
        , fake_socket * s )
    {
        assert( e.address().is_v4() );

        auto const index = e.address().to_v4().to_ulong();
        if ( index >= get_router().size() )
            get_router().resize( index + 1 );

        get_router().at( index ) = s;
    }

    /**
     *
     */
    static fake_socket *
    get_socket
        ( endpoint_type const& e )
    {
        assert( e.address().is_v4() );

        auto const index = e.address().to_v4().to_ulong();
        if ( index >= get_router().size() )
            return nullptr;

        return get_router().at( index );
    }

    /**
     *
     */
    static std::size_t
    copy_buffer
        ( boost::asio::const_buffer const& from
        , boost::asio::mutable_buffer const& to )
    {
        auto const source_size = boost::asio::buffer_size( from ); 
        auto source_data = boost::asio::buffer_cast< const uint8_t * >( from );
        auto target_data = boost::asio::buffer_cast< uint8_t * >( to );

        assert(  boost::asio::buffer_size( to ) <= source_size 
              && "can't store message into target buffer" );

        std::memcpy( target_data, source_data, source_size );

        return source_size;
    }

    /**
     *
     */
    template< typename Callback >
    void
    async_execute_write
        ( fake_socket * target
        , boost::asio::const_buffer const& buffer
        , Callback callback )
    { 
        auto perform_write = [ this, target, buffer, callback ] ( void )
        { 
            // Retrieve the read task of the packet.
            pending_read & t = target->pending_reads_.front();

            // Fill the read task buffer and endpoint.
            t.source_ = local_endpoint_;
            auto const copied_bytes_count = copy_buffer( buffer, t.buffer_ );

            // Inform the writer that data has been writeen.
            callback( boost::system::error_code()
                    , copied_bytes_count );

            // Inform the reader that data has been read.
            t.callback_( boost::system::error_code()
                       , copied_bytes_count );

            pending_reads_.pop_front();
        }; 

        io_service_.post( perform_write );
    }

    /**
     *
     */
    template< typename Callback >
    void
    async_execute_read
        ( boost::asio::mutable_buffer const& buffer
        , endpoint_type & from
        , Callback callback )
    { 
        auto perform_read = [ this, buffer, &from, callback ] ( void )
        {
            // Retrieve the write task of the packet.
            pending_write & t = pending_writes_.front();

            // Fill the provided buffer and endpoint.
            from  = t.source_;
            auto const copied_bytes_count = copy_buffer( t.buffer_, buffer ); 

            // Inform the readr that data has been readd.
            callback( boost::system::error_code()
                    , copied_bytes_count );

            // Now inform the writeer that data has been sent.
            t.callback_( boost::system::error_code()
                       , copied_bytes_count );

            // The current task has been consumed.
            pending_writes_.pop_front();
        };

        io_service_.post( perform_read );
    }

private:
    ///
    boost::asio::io_service & io_service_;
    ///
    endpoint_type local_endpoint_;
    ///
    std::deque< pending_read > pending_reads_;
    ///
    std::deque< pending_write > pending_writes_;
};

} // namespace kademlia

#endif

