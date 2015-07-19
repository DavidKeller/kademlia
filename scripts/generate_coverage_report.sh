#!/bin/sh

set -e

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
    echo "Usage: $0 PROJECT_ROOT" >&2
    exit -1
fi

project_root=`realpath $1`

# Build project.
CXXFLAGS='-fprofile-arcs -ftest-coverage' cmake ${project_root}
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
     --extract app_total.info '*kademlia/include*' '*kademlia/src*'

# Generate html report.
genhtml --output-directory html --prefix ${project_root} \
        --num-spaces 4 --title 'Kademlia unit tests' \
        app_total_stripped.info 

