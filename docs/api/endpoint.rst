Endpoint
========

**#include <kademlia/endpoint.hpp>**

.. cpp:class:: kademlia::endpoint

   Represents a network enpoint.

   This class is used to:

   * Provide the bootstraping participant **address** to the :cpp:class:`session`
   * Define the listening address(es) of a :cpp:class:`first_session` &
     :cpp:class:`session` instance

   .. rubric:: Constructors

   .. cpp:function:: endpoint \
                         ( void )

      Construct an unitialized endpoint.

   .. cpp:function:: endpoint \
                         ( address_type const& address \
                         , service_type const& service )

      Constructs an endpoint from an **address** and a port **service** identifier.

   .. cpp:function:: endpoint \
                         ( address_type const& address \
                         , service_numeric_type const& service )

       Constructs an endpoint from an **address** and a port **service** identifier.

   .. rubric:: Methods

   .. cpp:function:: address_type const& \
                     address \
                         ( void ) const          

      Get the **address** of the enpoint.

   .. cpp:function:: void \
                     address \
                         ( address_type const& address ) 

      Set the **address** of the enpoint.

   .. cpp:function:: service_type const& \
                     service \
                         ( void ) const          

      Get the **service** of the enpoint.

   .. cpp:function:: void \
                     service \
                         ( service_type const& service ) 

      Set the **service** of the enpoint.

   .. rubric:: Types

   .. cpp:type:: address_type = std::string

     Represents an address.

     It can be:

     * An IPv4 (e.g. :cpp:expr:`"192.168.1.1"`)
     * An IPv6 (e.g. :cpp:expr:`"2001:0db8:85a3:0000:0000:8a2e:0370:7334"`)
     * A hostname (e.g. :cpp:expr:`"host-7430"`)

   .. cpp:type:: service_type = std::string

      Represents a port.

      It can be:

      * An unsigned 16 bits integer (e.g. :cpp:expr:`"12345"`)
      * A protocol name (e.g. :cpp:expr:`"http"`)

   .. cpp:type:: service_numeric_type = std::uint16_t

      Represents a port as a 16 bits unsigned integer.

.. rubric:: Helpers

.. cpp:function:: std::ostream& \
                  operator<< \
                      ( std::ostream & out \
                      , endpoint const& e )

   Print the endpoint **e** string representation into the stream **out**.


.. cpp:function:: std::istream & \
                  operator>> \
                      ( std::istream & in \
                      , endpoint & e )

   Parse the endpoint **e** string representation from the stream **in**.

.. cpp:function:: bool \
                  operator== \
                      ( endpoint const& a \
                      , endpoint const& b )

    Compare endpoints **a** & **b** for equality.

    Return :cpp:expr:`true` if they are equal, :cpp:expr:`false` otherwise

.. cpp:function:: bool \
                  operator!= \
                      ( endpoint const& a \
                      , endpoint const& b )

    Compare endpoints **a** & **b** for inequality.

    Return :cpp:expr:`true` if they aren't equal, :cpp:expr:`false` otherwise
