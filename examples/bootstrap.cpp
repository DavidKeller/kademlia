#include <cstdint>
#include <cstdlib>

#include <future>
#include <iostream>
#include <iterator>
#include <sstream>

#include <kademlia/endpoint.hpp>
#include <kademlia/error.hpp>
#include <kademlia/first_session.hpp>

namespace k = kademlia;

int main
        ( int argc
        , char** argv )
{
    // Check command line arguments count
    if ( argc != 2 )
    {
        std::cerr << "usage: " << argv[0] << " <PORT>" << std::endl;
        return EXIT_FAILURE;
    }
        
    // Parse command line arguments
    std::uint16_t const port = std::atoi( argv[1] );

    // Create the session
    k::first_session session{ k::endpoint{ "0.0.0.0", port }
                            , k::endpoint{ "::", port } };

    // Start the main loop thread
    auto main_loop = std::async( std::launch::async
                               , &k::first_session::run, &session );

    // Wait for exit request
    std::cout << "Press any key to exit" << std::endl;
    std::cin.get();

    // Stop the main loop thread
    session.abort();

    // Wait for the main loop thread termination
    auto failure = main_loop.get();
    if ( failure != k::RUN_ABORTED )
        std::cerr << failure.message() << std::endl;
}
