#!/bin/bash

# Setup stxxl if not done already
# Reference: 
# https://stxxl.org/tags/1.4.1/install_unix.html
# https://stxxl.org/tags/1.4.1/tutorial_sorter.html
# https://stxxl.org/tags/1.4.1/design_algo_sort.html
if [ ! -d "stxxl" ]
then
git clone http://github.com/stxxl/stxxl.git stxxl
cd stxxl
mkdir build && cd build
cmake ..
make
cd local
./test1
pwd #.../stxxl/build/local
cp ../../examples/containers/sorter2.cpp ../../local
# rename
mv ../../local/test1.cpp ../../local/test1-backup.cpp
mv ../../local/sorter2.cpp ../../local/test1-sorter.cpp
mv ../../tests/io/test_io.cpp ../../local/test1-io.cpp
cp ~/cache_adaptivity/merge-sort/stxxl_sort.cpp ../../local/test1.cpp
make
# now change the memory given and data size in ../../local/test.cpp
fi