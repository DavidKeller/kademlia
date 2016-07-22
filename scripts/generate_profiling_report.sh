#!/bin/sh

set -e

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
    echo "Usage: $0 SOURCE_ROOT" >&2
    exit -1
fi

source_root=$(cd "$1"; pwd)

CXXFLAGS='-pg' cmake -DCMAKE_BUILD_TYPE=Release ${source_root}
make -j`nproc`

time test/simulator/simulator --clients-count=50 --messages-count=10000 

gprof test/simulator/simulator | gprof2dot --strip --node-thres=5 | dot -Tpng -o profile.png

