# SPDX-License-Identifier: MIT

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)

# Threads
find_package(Threads REQUIRED)

# Boost
set(boost_components
  system
  filesystem
)
if(KADEMLIA_BUILD_TEST)
  list(APPEND boost_components unit_test_framework)
endif()
find_package(Boost CONFIG REQUIRED COMPONENTS ${boost_components})

# Crypto
find_package(OpenSSL CONFIG REQUIRED)
