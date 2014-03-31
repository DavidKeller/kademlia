# Description
C++11 distributed hash table library

## Example

The library is so simple that one light header is enough.
```C++
#include <kademlia/session.hpp>
```

A session instance has to be created.
```C++
// ...

{
    // The session need to know at least one member of the
    // distributed table to find neighbors.
    kademlia::endpoint const initial_peer{ "1.2.3.4", 27980 };
    
    // If an error occurs, this will throw.
    // Following runtime errors will be reported
    // through an std::error_code.
    kademlia::session s{ initial_peer };
```

Then a search for the key `"keepsake"` can be performed.
```
    // This callback will be called once the load succeed or failed.
    auto on_load_finished = [&s]( std::error_code const& error
                                , kademlia::session::data_type const& data )
    { 
        if ( failure )
            std::cerr << failure.message() << std::endl;
        else
            std::copy( data.begin(), data.end()
                     , std::ostream_iterator< std::uint32_t >{ std::cout, " " } );
            
        // Stop running loop, we won't load anything else in this example.
        s.abort();
    };
    
    // Schedule an asynchronous load.
    s.async_load( "keepsake", on_load_finished );
```

The library event loop can run in a dedicated thread but for
the example purpose, we'll run it directly from the current thread
without problem as `async_load` and `async_save` methods 
are asynchronous.
```C++
    // Run the library main loop. It will exit soon as the
    // on_load_finish callback abort dispatching when called.
    s.run();
}
```

# Development
* Documentation intended for developers is stored under 'doc/'.
* [Bug tracker](http://redmine.litchis.fr/projects/kademlia)

## Supported targets
FreeBSD 9 | Fedora 19 | Ubuntu 13 | Windows 7
----------|-----------|-----------|----------
[![FreeBSD9 build](http://buildbot.litchis.fr/png?builder=freebsd9-x64-builder)](http://buildbot.litchis.fr/builders/freebsd9-x64-builder) | [![Fedora19 build](http://buildbot.litchis.fr/png?builder=fedora19-x64-builder)](http://buildbot.litchis.fr/builders/fedora19-x64-builder) | [![Ubuntu build](http://buildbot.litchis.fr/png?builder=ubuntu13-x64-builder)](http://buildbot.litchis.fr/builders/ubuntu13-x64-builder) | [![Windows 7 build](http://buildbot.litchis.fr/png?builder=win2008r2-x64-builder)](http://buildbot.litchis.fr/builders/win2008r2-x64-builder)
