Session
=======

**#include <kademlia/session.hpp>**

.. cpp:class:: kademlia::session

   This class is used to save and load data from the network.

   .. note:: Session instances can be *moved* but can't be *copied*.

   .. rubric:: Constructors

   .. cpp:function:: session \
                         ( endpoint const& initial_peer \
                         , endpoint const& listen_on_ipv4 = endpoint( "0.0.0.0", DEFAULT_PORT ) \
                         , endpoint const& listen_on_ipv6 = endpoint( "::", DEFAULT_PORT ) )

      Constructs an active session from an **initial_peer**.

      .. attention::

         This session perform a neighbors discovery on creation.
         If the network is down or the **initial_peer** can't be contacted,
         an exception will be throw.

      Both **listen_on_ipv4** and **listen_on_ipv6** can be provided to
      change the addresses and ports this session is using to exchange
      with other peers.

   .. rubric:: Methods

   .. cpp:function:: void \
                     async_save \
                         ( key_type const& key \
                         , data_type const& data \
                         , save_handler_type handler )

      Asynchronously save a **data** with **key** within the network.
      On completion, the provided **handler** is called.

   .. cpp:function:: template< typename KeyType, typename DataType > \
                     void \
                     async_save \
                         ( KeyType const& key \
                         , DataType const& data \
                         , save_handler_type handler )

      This methods acts like :cpp:func:`session::async_save()` but
      accepts any *bytes* sequence as **key** and **data**.

   .. cpp:function:: void \
                     async_load \
                         ( key_type const& key \
                         , load_handler_type handler )

      Asynchronously retrieve the data associated with **key** within the network.
      On completion, the provided **handler** is called, with the associated data
      in case of success.

   .. cpp:function:: template< typename KeyType > \
                     void \
                     async_load \
                         ( KeyType const& key \
                         , save_handler_type handler )

      This methods acts like :cpp:func:`session::async_load()` but
      accepts any *bytes* sequence as **key**.

   .. cpp:function:: std::error_code \
                     run \
                         ( void )

      This blocking call executes the :cpp:class:`session` main loop.

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

      Abort the :cpp:class:`session` main loop, that is make the
      :cpp:func:`run()` call exit.

   .. rubric:: Members

   .. cpp:member:: constexpr std::uint16_t DEFAULT_PORT = 27980

      The default port used by the session

   .. rubric:: Types

   .. cpp:type:: data_type = std::vector< std::uint8_t >

      Represents the data as a buffer of bytes.

   .. cpp:type:: key_type = std::vector< std::uint8_t >

      Represents the key as a buffer of bytes.

   .. cpp:type:: save_handler_type 

      Represents the handler called by the :cpp:func:`async_save()` method.

      It can be any function or functor with the following signature:
      :cpp:expr:`void ( std::error_code const& error )`

   .. cpp:type:: load_handler_type 

      Represents the handler called by the :cpp:func:`async_load()` method.

      It can be any function or functor with the following signature:
      :cpp:expr:`void ( std::error_code const& error, data_type const& data )`

