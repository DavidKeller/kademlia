# Module for locating the Botan cryptographic library.
#
# Customizable variables:
#   BOTAN_ROOT_DIR
#     This variable points to the Botan root directory. On Windows the
#     library location typically will have to be provided explicitly using the
#     -D command-line option.
#
# Read-only variables:
#   BOTAN_FOUND
#     Indicates whether the library has been found.
#
#   BOTAN_INCLUDE_DIRS
#     Points to the Botan include directory.
#
#   BOTAN_LIBRARIES
#     Points to the Botan libraries that should be passed to
#     target_link_libararies.
#
#
# Copyright (c) 2012 Sergiu Dotenco
#               2014 David Keller
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

INCLUDE (FindPackageHandleStandardArgs)

SET (_BOTAN_POSSIBLE_LIB_PATH_SUFFIXES lib)
SET (_BOTAN_POSSIBLE_VERSION 1.15 1.14 1.13 1.12 1.11 1.10 1.9)

IF ("${BOTAN_ROOT_DIR}" STREQUAL "")
    set(BOTAN_ROOT_DIR $ENV{BOTAN_ROOT})
ENDIF ("${BOTAN_ROOT_DIR}" STREQUAL "")

FOREACH (_BOTAN_VER ${_BOTAN_POSSIBLE_VERSION})
    LIST (APPEND _BOTAN_INCLUDE_SUFFIXES "include/botan-${_BOTAN_VER}")
ENDFOREACH (_BOTAN_VER ${_BOTAN_POSSIBLE_VERSION})

FIND_PATH (BOTAN_INCLUDE_DIR
  NAMES botan/botan.h
  HINTS ${BOTAN_ROOT_DIR}
  PATH_SUFFIXES include ${_BOTAN_INCLUDE_SUFFIXES}
  DOC "Botan include directory")

IF (BOTAN_INCLUDE_DIR)
  SET (_BOTAN_VERSION_HEADER ${BOTAN_INCLUDE_DIR}/botan/build.h)

  IF (EXISTS ${_BOTAN_VERSION_HEADER})
    FILE (STRINGS ${_BOTAN_VERSION_HEADER} _BOTAN_VERSION_TMP REGEX
      "#define BOTAN_VERSION_(MAJOR|MINOR|PATCH)[ \t]+[0-9]+")

    STRING (REGEX REPLACE
      ".*#define BOTAN_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" BOTAN_VERSION_MAJOR
      ${_BOTAN_VERSION_TMP})
    STRING (REGEX REPLACE
      ".*#define BOTAN_VERSION_MINOR[ \t]+([0-9]+).*" "\\1" BOTAN_VERSION_MINOR
      ${_BOTAN_VERSION_TMP})
    STRING (REGEX REPLACE
      ".*#define BOTAN_VERSION_PATCH[ \t]+([0-9]+).*" "\\1" BOTAN_VERSION_PATCH
      ${_BOTAN_VERSION_TMP})

    SET (BOTAN_VERSION_COUNT 3)
    SET (BOTAN_VERSION
      ${BOTAN_VERSION_MAJOR}.${BOTAN_VERSION_MINOR}.${BOTAN_VERSION_PATCH})
  ENDIF (EXISTS ${_BOTAN_VERSION_HEADER})
ENDIF (BOTAN_INCLUDE_DIR)

SET (_BOTAN_LIBRARY_SUFFIX "${BOTAN_VERSION_MAJOR}.${BOTAN_VERSION_MINOR}")

FIND_LIBRARY (BOTAN_LIBRARY_DEBUG
  NAMES botand botand-${_BOTAN_LIBRARY_SUFFIX}
  HINTS ${BOTAN_ROOT_DIR}
  PATH_SUFFIXES ${_BOTAN_POSSIBLE_LIB_PATH_SUFFIXES}
  DOC "Botan debug library")

FIND_LIBRARY (BOTAN_LIBRARY_RELEASE
  NAMES botan botan-${_BOTAN_LIBRARY_SUFFIX}
  HINTS ${BOTAN_ROOT_DIR}
  PATH_SUFFIXES ${_BOTAN_POSSIBLE_LIB_PATH_SUFFIXES}
  DOC "Botan release library")

IF (BOTAN_LIBRARY_DEBUG AND BOTAN_LIBRARY_RELEASE)
  SET (BOTAN_LIBRARY
    optimized ${BOTAN_LIBRARY_RELEASE}
    debug ${BOTAN_LIBRARY_DEBUG} CACHE DOC "Botan library")
ELSEIF (BOTAN_LIBRARY_RELEASE)
  SET (BOTAN_LIBRARY ${BOTAN_LIBRARY_RELEASE} CACHE DOC "Botan library")
ENDIF (BOTAN_LIBRARY_DEBUG AND BOTAN_LIBRARY_RELEASE)

SET (BOTAN_INCLUDE_DIRS ${BOTAN_INCLUDE_DIR})
SET (BOTAN_LIBRARIES ${BOTAN_LIBRARY})

MARK_AS_ADVANCED (BOTAN_INCLUDE_DIR BOTAN_LIBRARY
  BOTAN_LIBRARY_DEBUG BOTAN_LIBRARY_RELEASE)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (Botan REQUIRED_VARS 
  BOTAN_INCLUDE_DIR BOTAN_LIBRARY VERSION_VAR BOTAN_VERSION)
