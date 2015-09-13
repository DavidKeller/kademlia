#!/bin/sh

set -e

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
    echo "Usage: $0 SOURCE_ROOT" >&2
    exit -1
fi

source_root=`realpath $1`

# Build project.
CXXFLAGS='--coverage' cmake -DCMAKE_BUILD_TYPE=Debug ${source_root}
make -j`nproc`

# Generate initial coverage file.
lcov --quiet --output-file app_base.info \
     --initial --capture --directory .

# Run tests.
make -j`nproc` check

# Generate post test coverage file.
lcov --quiet --output-file app_test.info \
     --capture --directory .

# Merge coverage files.
lcov --quiet --output-file app_total.info \
     --add-tracefile app_base.info \
     --add-tracefile app_test.info

# Clean coverage file.
lcov --quiet  --output-file app_total_stripped.info \
     --extract app_total.info '*include/kademlia/*' '*src/kademlia/*'

# Generate html report.
genhtml --output-directory html --prefix ${source_root} \
        --num-spaces 4 --title 'Kademlia unit tests' \
        --no-function-coverage \
        app_total_stripped.info 

