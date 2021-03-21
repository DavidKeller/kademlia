Walk-through
============

.. cpp:namespace:: kademlia

In order to join the network, the C++ :cpp:class:`session` class should be
instantiated.

This class provides the :cpp:func:`session::async_save()` &
:cpp:func:`session::async_load()` methods used to respectively store *key*
with its associated *value* within the network and to retrieve the *value*
associated with a *key*.

Due to the network nature, these methods are asynchronous and take as
an argument the callback to call once the request has completed.
This design allows an application to execute multiple requests in parallel
and hide network latency by performing other tasks in the meantime
These callbacks will be executed within the context of the :cpp:class:`session`
main event loop which is :cpp:func:`session::run()`.
An application should call this :cpp:func:`session::run()` within a dedicated
thread (and will have to use some synchronization mechanism like mutex
within the callbacks when interacting with data touched by other threads).

The :cpp:class:`session` constructor requires the network address of one
participant, this is mandatory in order to discover other participant.
This first participant is a kind of go-between.

A skilled reader may wonder what participant address should be provided to
the first participant of the network?
The :cpp:class:`first_session` is dedicated
to this use case, and its constructor doesn't require any participant address.
Once there is at least one other participant within the network, the participant
using the :cpp:class:`first_session` class may leave, the network will still be
functional.
The :cpp:class:`first_session` doesn't act as a server, it's only an ephemeral actor
used to bootstrap the network.
