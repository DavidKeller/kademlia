First Session
=============

**#include <kademlia/first_session.hpp>**

.. cpp:class:: kademlia::first_session

   This class is used to bootstrap a network.

   .. note:: First session instances can be *moved* but can't be *copied*.

   .. rubric:: Constructors

   .. cpp:function:: first_session \
                         ( endpoint const& listen_on_ipv4 = endpoint( "0.0.0.0", DEFAULT_PORT ) \
                         , endpoint const& listen_on_ipv6 = endpoint( "::", DEFAULT_PORT ) )

      Constructs a passive session.

      .. note::

         This first_session acts like an active :cpp:class:`session` except it
         does'nt try to discover neighbors. It can be used by the first node
         of a network as no peer is known uppon its creation.

      Both **listen_on_ipv4** and **listen_on_ipv6** can be provided to
      change the addresses and ports this session is using to exchange
      with other peers.

   .. rubric:: Methods

   .. cpp:function:: std::error_code \
                     run \
                         ( void )

      This blocking call executes the :cpp:class:`first_session` main loop.

      This method should be called in a dedicated thread.

      .. attention::

         Exception can be thrown from this method.

         These can be library internal exceptions (e.g. failure to reach
         initial_peer) or exceptions coming from the user-provided handlers.

         Remember that if exception reaches a thread main funtion, the
         application will abort.
         Hence this method invocation should be wrapped within a
         :cpp:expr:`std::packaged_task<>` (e.g. by using
         :cpp:expr:`std::async()`).

      In order to exit from the main loop, :cpp:func:`abort()` should be
      called.

      The exit reason is returned.

   .. cpp:function:: void \
                     abort \
                         ( void )

      Abort the :cpp:class:`first_session` main loop, that is make the
      :cpp:func:`run()` call exit.

   .. rubric:: Members

   .. cpp:member:: constexpr std::uint16_t DEFAULT_PORT = 27980

      The default port used by the session

