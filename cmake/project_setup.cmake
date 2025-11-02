# SPDX-License-Identifier: MIT

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_CTEST_ARGUMENTS --output-on-failure --timeout 10)
enable_testing()

add_compile_definitions(
    PACKAGE_VERSION="${PROJECT_VERSION}"
    PACKAGE_BUGREPORT="david.keller@litchis.fr"
    $<$<CONFIG:Debug>:KADEMLIA_ENABLE_DEBUG>
    $<$<CXX_COMPILER_ID:MSVC>:_WIN32_WINNT=_WIN32_WINNT_WIN10>
)
add_compile_options(
    $<$<CXX_COMPILER_ID:Clang>:-stdlib=libc++>
)