# Copyright (c) 2014, David Keller
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the University of California, Berkeley nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY DAVID KELLER AND CONTRIBUTORS ``AS IS AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

if(PythonBehave AND PYTHONLIBS_FOUND AND Boost_PYTHON_FOUND)

include_directories(${PYTHON_INCLUDE_DIRS})

add_custom_target(chain_tests COMMAND behave --logging-level=DEBUG --no-color ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(check chain_tests)
add_dependencies(chain_tests _kademlia)

# Append output directory path to python search paths.
set(ENV{PYTHONPATH} ${CMAKE_CURRENT_BINARY_DIR}:$ENV{PYTHONPATH})

add_library(_kademlia SHARED
    _kademlia.cpp)

set_target_properties(_kademlia
    PROPERTIES
        MACOSX_RPATH ON
        SUFFIX ".so"
        PREFIX "")

target_link_libraries(_kademlia
    kademlia_static
    ${Boost_PYTHON_LIBRARY} 
    ${PYTHON_LIBRARIES} 
    ${CMAKE_THREAD_LIBS_INIT})

else()
    message("++ Disabling chain tests due to lack of:")
    if(NOT PythonBehave)
        message("++   python-behave")
    endif()
    if(NOT PYTHONLIBS_FOUND)
        message("++   python devel")
    endif()
    if(NOT Boost_PYTHON_FOUND)
        message("++   boost-python")
    endif()
endif()

