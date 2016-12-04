[![Build Status](https://travis-ci.org/DavidKeller/kademlia.svg?branch=master)](https://travis-ci.org/DavidKeller/kademlia)
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
<tr><th>OS</th><th>Compiler</th><th>Build + Unit tests</th></tr>
<tr><td>FreeBSD 9</td><td>clang</td><td><a href="http://buildbot.litchis.fr/kademlia/builders/freebsd9-x64-builder"><img src="http://buildbot.litchis.fr/kademlia/png?builder=freebsd9-x64-builder" /></a></td></tr>
<tr><td>Fedora 21</td><td>gcc</td><td><a href="http://buildbot.litchis.fr/kademlia/builders/fedora21-x64-builder"><img src="http://buildbot.litchis.fr/kademlia/png?builder=fedora21-x64-builder" /></a></td></tr>
<tr><td>Ubuntu 14</td><td>gcc</td><td><a href="http://buildbot.litchis.fr/kademlia/builders/ubuntu14-x64-builder"><img src="http://buildbot.litchis.fr/kademlia/png?builder=ubuntu14-x64-builder" /></a></td></tr>
<tr><td>Centos 7</td><td>gcc</td><td><a href="http://buildbot.litchis.fr/kademlia/builders/centos7-x64-builder"><img src="http://buildbot.litchis.fr/kademlia/png?builder=centos7-x64-builder" /></a></td></tr>
<tr><td>Debian 8</td><td>gcc</td><td><a href="http://buildbot.litchis.fr/kademlia/builders/debian8-x64-builder"><img src="http://buildbot.litchis.fr/kademlia/png?builder=debian8-x64-builder" /></a></td></tr>
<tr><td>Windows 7</td><td>MSVC</td><td><a href="http://buildbot.litchis.fr/kademlia/builders/win2008r2-x64-builder"><img src="http://buildbot.litchis.fr/kademlia/png?builder=win2008r2-x64-builder" /></a></td></tr>
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

