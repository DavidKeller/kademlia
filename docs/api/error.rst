Error
=====

**#include <kademlia/error.hpp>**

.. cpp:enum:: kademlia::error_type

   Represents all the library specific errors

   .. cpp:enumerator:: UNKNOWN_ERROR = 1

      An unknown error.

   .. cpp:enumerator:: RUN_ABORTED

      The :cpp:func:`session::abort()` has been called.

   .. cpp:enumerator:: INITIAL_PEER_FAILED_TO_RESPOND

      The session failed to contact a valid peer uppon creation.

   .. cpp:enumerator:: MISSING_PEERS

      The session routing table is missing peers.

   .. cpp:enumerator:: INVALID_ID

      An id has been corrupted.

   .. cpp:enumerator:: TRUNCATED_ID

      An id has been truncated.

   .. cpp:enumerator:: TRUNCATED_HEADER

      A packet header from the network is corrupted.

   .. cpp:enumerator:: TRUNCATED_ENDPOINT

      An endpoint information has been corrupted.

   .. cpp:enumerator:: TRUNCATED_ADDRESS

      An endpoint address has been corrupted.

   .. cpp:enumerator:: TRUNCATED_SIZE

      A list has been corrupted.

   .. cpp:enumerator:: UNKNOWN_PROTOCOL_VERSION

      A message from an unknown version of the library has been received.

   .. cpp:enumerator:: CORRUPTED_BODY

      A packet body has been corrupted.

   .. cpp:enumerator:: UNASSOCIATED_MESSAGE_ID

      An unexpected response has been received.

   .. cpp:enumerator:: INVALID_IPV4_ADDRESS

      The provided IPv4 address is invalid.

   .. cpp:enumerator:: INVALID_IPV6_ADDRESS

      The provided IPv6 address is invalid.

   .. cpp:enumerator:: UNIMPLEMENTED

      The function/method has been implemented yet.

   .. cpp:enumerator:: VALUE_NOT_FOUND

      The value associated with the requested key has not been found.

   .. cpp:enumerator:: TIMER_MALFUNCTION

      The internal timer failed to tick.

   .. cpp:enumerator:: ALREADY_RUNNING

      Another call to :cpp:func:`session::run()` is still blocked.
