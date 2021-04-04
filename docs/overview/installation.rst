Installation
============

Requirements
------------

* **CMake** >= **3.5**
* **Boost** >= **1.65**

BSD or Linux build
------------------

To build from this project root directory:

.. code-block:: shell

    $ mkdir build; cd build
    $ cmake ..
    $ make -j `nproc`
    $ make check

Inclusion into existing CMake project
-------------------------------------

The project can be integrated into another **CMake** project using
``add_directory()`` to target the local checkout of this **kademlia** project.

**Existing project CMakeLists.txt**:

.. code-block:: cmake

   project(ExistingProject) 

   add_subdirectory(PATH/TO/KAMDELIA_PROJECT/LOCAL_CHECKOUT)

   add_executable(app main.cpp)

   # The kademlia target can be referenced directly
   add_library(app kademlia)
