# Copyright (c) 2013, David Keller
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
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(kademlia)

# Configure compilers.
if(WIN32)
    if (MSVC_VERSION LESS 1800) # 1800 -> VS 12.0
        message(FATAL_ERROR "Current compiler does'nt support c++11 yet")
    endif()
    add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WIN7
        -D_WINSOCK_DEPRECATED_NO_WARNINGS)
else()
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-std=c++11 IS_CXX11_SUPPORTED)
    if(IS_CXX11_SUPPORTED)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
    else()
        message(FATAL_ERROR "Current compiler does'nt support c++11 yet")
    endif()
    check_cxx_compiler_flag(-stdlib=libc++ IS_LIBCPP_PRESENT)
    if(IS_LIBCPP_PRESENT AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()


# Check environment.
# Threads
find_package(Threads REQUIRED)
# Boost
if(WIN32)
    set(Boost_USE_STATIC_LIBS ON)
#elseif(APPLE)
#    set(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost REQUIRED COMPONENTS
    system
    filesystem
    program_options
    unit_test_framework)
# Crypto
find_package(OpenSSL REQUIRED)

add_definitions(-DPACKAGE_VERSION="0.0.0")
add_definitions(-DPACKAGE_BUGREPORT="david.keller@litchis.fr")
add_definitions(-DBOOST_ALL_NO_LIB
    -DBOOST_ASIO_HAS_MOVE
    -DBOOST_ASIO_HAS_VARIADIC_TEMPLATES
    -DBOOST_ASIO_HAS_STD_SYSTEM_ERROR
    -DBOOST_ASIO_HAS_STD_ARRAY
    -DBOOST_ASIO_HAS_STD_SHARED_PTR
    -DBOOST_ASIO_HAS_STD_ATOMIC
    -DBOOST_ASIO_HAS_STD_CHRONO)
if(NOT Boost_USE_STATIC_LIBS)
    add_definitions(-DBOOST_ALL_DYN_LINK)
endif()
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DKADEMLIA_ENABLE_DEBUG)
endif()

# Setup includes directories.
include_directories(BEFORE include src test)
include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})

# Build everything.
add_subdirectory(include)
add_subdirectory(src)
enable_testing()
add_subdirectory(test)

