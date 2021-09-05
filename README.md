[![Documentation Status](https://readthedocs.org/projects/kademlia-cpp/badge/?version=master)](https://kademlia-cpp.readthedocs.io/en/latest/?badge=master)
[![Build Status Linux](https://app.travis-ci.com/DavidKeller/kademlia.svg?branch=master)](https://app.travis-ci.com/DavidKeller/kademlia)
[![Build status Windows](https://ci.appveyor.com/api/projects/status/vf43m6qq8fk6kri1?svg=true)](https://ci.appveyor.com/project/DavidKeller/kademlia)
[![Coverage Status](https://coveralls.io/repos/github/DavidKeller/kademlia/badge.svg?branch=master)](https://coveralls.io/github/DavidKeller/kademlia?branch=master)

# Description
C++11 distributed hash table library

**This software is experimental and under active development**.

# Development

## Bug and feature requests
* [Tracker](http://redmine.litchis.fr/projects/kademlia)

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

