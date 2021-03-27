Command Line Interface
======================

The following example demonstrates a console application that is capable
of saving and loading data from/to a Kademlia network.

The save & load requests are read from **stdin**. Depending on the request
type, either **load()** or **save()** is called.

.. literalinclude:: ../../examples/cli.cpp
   :language: cpp
   :linenos:
