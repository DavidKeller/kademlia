[![Documentation Status](https://readthedocs.org/projects/kademlia-cpp/badge/?version=latest)](https://kademlia-cpp.readthedocs.io/en/latest/?badge=latest)
[![Build Status Linux](https://travis-ci.org/DavidKeller/kademlia.svg?branch=master)](https://travis-ci.org/DavidKeller/kademlia)
[![Build status Windows](https://ci.appveyor.com/api/projects/status/vf43m6qq8fk6kri1?svg=true)](https://ci.appveyor.com/project/DavidKeller/kademlia)
[![Coverage Status](https://coveralls.io/repos/github/DavidKeller/kademlia/badge.svg?branch=master)](https://coveralls.io/github/DavidKeller/kademlia?branch=master)


# Description
C++11 distributed hash table library

**This software is experimental and under active development**.

### Example
Initialization:
```C++
#include <kademlia/session.hpp>

// [...]

{
    // The session need to know at least one member of the
    // distributed table to find neighbors.
    kademlia::endpoint const initial_peer{ "1.2.3.4", 27980 };

    // If an error occurs, this will throw.
    // Following runtime errors will be reported
    // through an std::error_code.
    kademlia::session s{ initial_peer };

    // Run the library main loop in a dedicated thread.
    auto main_loop_result = std::async( std::launch::async
                                      , &kademlia::session::run, &s );

    // [...]
```

Searching for value associated with "key1":
```C++
    // This callback will be called once the load succeed or failed.
    auto on_load = []( std::error_code const& failure
                     , kademlia::session::data_type const& data )
    {
        if ( failure )
            std::cerr << failure.message() << std::endl;
        else
            std::copy( data.begin(), data.end()
                     , std::ostream_iterator< std::uint32_t >{ std::cout, " " } );
    };

    // Schedule an asynchronous load.
    s.async_load( "key1", on_load );

    // [...]
```

Saving a data into the table is similar:
```C++
    // Copy data from your source.
    kademlia::session::data_type const data{ ?.begin(), ?.end() };

    // Create the handler.
    auto on_save = []( std::error_code const& failure )
    {
        if ( failure )
            std::cerr << failure.message() << std::endl;
    }

    // And perform the saving.
    s.async_save( "key2", data, on_save );

    // [...]
```

Destruction:
```C++
    // Once we've finished, abort the session::run()
    // blocking call.
    s.abort();

    auto failure = main_loop_result.get();
    if ( failure != kademlia::RUN_ABORTED )
        std::cerr << failure.message() << std::endl;
}
```

# Development

### Bug and feature requests
* [Tracker](http://redmine.litchis.fr/projects/kademlia)

### Supported targets
<table>
<tr><th>OS</th><th>Compiler</th></tr>
<tr><td>FreeBSD</td><td>clang</td></tr>
<tr><td>Fedora</td><td>gcc</td></tr>
<tr><td>Ubuntu</td><td>gcc</td></tr>
<tr><td>Centos</td><td>gcc</td></tr>
<tr><td>Debian</td><td>gcc</td></tr>
<tr><td>Windows 7+</td><td>MSVC</td></tr>
</table>

### Project structure
```
kademlia/
    |
    |-- include             API public headers files.
    |-- src                 Implementation sources and headers files.
    |-- test/unit_tests     Unit tests.
    |-- test/simulator      Chain test application.
    |-- doc                 User and development documentation.
```

