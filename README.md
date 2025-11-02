[![Documentation Status](https://readthedocs.org/projects/kademlia-cpp/badge/?version=master)](https://kademlia-cpp.readthedocs.io/en/latest/?badge=master)
[![Build status Linux](https://github.com/DavidKeller/kademlia/actions/workflows/linux.yaml/badge.svg)](https://github.com/DavidKeller/kademlia/actions/workflows/linux.yaml)
[![Build status OSX](https://github.com/DavidKeller/kademlia/actions/workflows/osx.yaml/badge.svg)](https://github.com/DavidKeller/kademlia/actions/workflows/osx.yaml)
[![Build status Windows](https://github.com/DavidKeller/kademlia/actions/workflows/windows.yaml/badge.svg)](https://github.com/DavidKeller/kademlia/actions/workflows/windows.yaml)
[![Coverage Status](https://coveralls.io/repos/github/DavidKeller/kademlia/badge.svg?branch=master)](https://coveralls.io/github/DavidKeller/kademlia?branch=master)

# Description
C++11 distributed hash table library

**This software is experimental and under development**.

# Development

## Supported targets
<table>
<tr><th>OS</th><th>Compiler</th></tr>
<tr><td>OSX</td><td>clang</td></tr>
<tr><td>NIX</td><td>gcc</td></tr>
<tr><td>Windows 7+</td><td>MSVC</td></tr>
</table>

## Project structure
```
kademlia/
    |
    |-- include             API public headers files.
    |-- src                 Implementation sources and headers files.
    |-- test/unit_tests     Unit tests.
    |-- docs                User and development documentation.
    |-- specs               Kademlia protocol papers
```

