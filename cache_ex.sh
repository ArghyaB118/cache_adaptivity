#!/bin/bash
g++ binary-merge-sort.cpp -o ./executables/binary-merge-sort
sudo chmod a+x ./executables/binary-merge-sort
g++ k-way-merge-sort.cpp -o ./executables/k-way-merge-sort
sudo chmod a+x ./executables/k-way-merge-sort
g++ k-way-merge-sort-cgroup.cpp -o ./executables/k-way-merge-sort-cgroup
sudo chmod a+x ./executables/k-way-merge-sort-cgroup
g++ funnel_sort.cpp -o ./executables/funnel_sort
sudo chmod a+x ./executables/funnel_sort
g++ cache_adaptive.cpp -o ./executables/cache-adaptive
sudo chmod a+x ./executables/cache-adaptive
g++ non_cache_adaptive.cpp -o ./executables/non-cache-adaptive
sudo chmod a+x ./executables/non-cache-adaptive


if [ -d  "/var/cgroups/$1" ]
then
	cgdelete memory:$1
fi

cgcreate -g "memory:$1" -t arghya:arghya

sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$1/memory.oom_control"

