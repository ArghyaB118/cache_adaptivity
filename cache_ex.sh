#!/bin/bash
#how-to-tun: sudo ./cache-ex.sh 32 128 cache-test-arghya
g++ ./merge-sort/binary-merge-sort.cpp -o ./executables/binary-merge-sort
chmod a+x ./executables/binary-merge-sort
g++ ./merge-sort/k-way-merge-sort.cpp -o ./executables/k-way-merge-sort
chmod a+x ./executables/k-way-merge-sort
g++ ./merge-sort/k-way-merge-sort-constant-memory.cpp -o ./executables/k-way-merge-sort-constant-memory
chmod a+x ./executables/k-way-merge-sort-constant-memory
g++ ./merge-sort/k-way-merge-sort-worst-case-memory.cpp -o ./executables/k-way-merge-sort-worst-case-memory
chmod a+x ./executables/k-way-merge-sort-worst-case-memory
g++ ./funnel-sort/funnel_sort.cpp -o ./executables/funnel_sort
chmod a+x ./executables/funnel_sort
g++ ./matrix-mul/cache_adaptive.cpp -o ./executables/cache-adaptive
chmod a+x ./executables/cache-adaptive
g++ ./matrix-mul/non_cache_adaptive.cpp -o ./executables/non-cache-adaptive
chmod a+x ./executables/non-cache-adaptive


if [ -d  "/var/cgroups/$1" ]
then
	cgdelete memory:$1
fi

cgcreate -g "memory:$1" -t arghya:arghya

g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data
./executables/make-unsorted-data $2

sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

cgexec -g memory:$3 ./executables/k-way-merge-sort-constant-memory $1 $2 $3

sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

cgexec -g memory:$3 ./executables/funnel_sort $1 $2 $3

sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

cgexec -g memory:$3 ./executables/k-way-merge-sort-worst-case-memory $1 $2 $3


