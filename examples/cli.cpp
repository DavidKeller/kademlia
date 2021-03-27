#include <cstdint>
#include <cstdlib>

#include <future>
#include <iostream>
#include <iterator>
#include <sstream>

#include <kademlia/endpoint.hpp>
#include <kademlia/session.hpp>
#include <kademlia/error.hpp>

namespace k = kademlia;

namespace {

const char HELP[] =
"save <KEY> <VALUE>\n\tSave <VALUE> as <KEY>\n\n"
"load <KEY>\n\tLoad value associated with <KEY>\n\n"
"help\n\tPrint this message\n\n";

std::vector< std::string >
split( std::string const& line )
{
    std::istringstream in{ line };

    using iterator = std::istream_iterator< std::string >;
    return std::vector< std::string >{ iterator{ in }, iterator{} };
}

void
load( k::session & session
    , std::string const& key )
{
    auto on_load = [ key ] ( std::error_code const& error
                           , k::session::data_type const& data )
    {
        if ( error )
            std::cerr << "Failed to load \"" << key << "\"" << std::endl;
        else
        {
            std::string const& str{ data.begin(), data.end() };
            std::cout << "Loaded \"" << key << "\" as \""
                      << str << "\"" << std::endl;
        }
    };

    session.async_load( key, std::move( on_load ) );
}

void
save( k::session & session
    , std::string const& key
    , std::string const& value )
{
    auto on_save = [ key ] ( std::error_code const& error )
    {
        if ( error )
            std::cerr << "Failed to save \"" << key << "\"" << std::endl;
        else
            std::cout << "Saved \"" << key << "\"" << std::endl;
    };

    session.async_save( key, value, std::move( on_save ) );

}

void
print_interactive_help
        ( void )
{
    std::cout << HELP << std::flush;
}

} // anonymous namespace

int main
        ( int argc
        , char** argv )
{
    // Check command line arguments count
    if ( argc != 3 )
    {
        std::cerr << "usage: " << argv[0] << " <PORT> <INITIAL_PEER>" << std::endl;
        return EXIT_FAILURE;
    }
        
    // Parse command line arguments
    std::uint16_t const port = std::atoi( argv[1] );
    k::endpoint initial_peer;
    std::istringstream{ argv[2] } >> initial_peer;

    // Create the session
    k::session session{ initial_peer
                      , k::endpoint{ "0.0.0.0", port }
                      , k::endpoint{ "::", port } };

    // Start the main loop thread
    auto main_loop = std::async( std::launch::async
                               , &k::session::run, &session );

    // Parse stdin until EOF (CTRL-D in Unix, CTRL-Z-Enter on Windows))
    std::cout << "Enter \"help\" to see available actions" << std::endl;
    for ( std::string line; std::getline( std::cin, line ); )
    {
        auto const tokens = split( line );

        if ( tokens.empty() )
            continue;

        if ( tokens[0] == "help" )
        {
            print_interactive_help();
        }
        else if ( tokens[0] == "save" )
        {
            if ( tokens.size() != 3 )
                print_interactive_help();
            else
                save( session, tokens[1], tokens[2] );
        }
        else if ( tokens[0] == "load" )
        {
            if ( tokens.size() != 2 )
                print_interactive_help();
            else
                load( session, tokens[1] );
        }
        else
            print_interactive_help();
    }

    // Stop the main loop thread
    session.abort();

    // Wait for the main loop thread termination
    auto failure = main_loop.get();
    if ( failure != k::RUN_ABORTED )
        std::cerr << failure.message() << std::endl;
}
