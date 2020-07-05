#!/bin/bash
set -ex
g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data
g++ ./merge-sort/test.cpp -o ./executables/test
chmod a+x ./executables/test

#creating nullbytes
if [ ! -f "merge-sort/nullbytes" ]
then
  echo "First creating file for storing data."
  dd if=/dev/urandom of=nullbytes count=32768 bs=1048576
  mv nullbytes merge-sort/nullbytes
fi
#deleting out-sorting.txt and creating again
if [ ! -f "out-sorting.txt" ]
then
	mkdir out-sorting.txt
fi
echo "---- new run ----" >> out-sorting.txt

declare -a data_size=( 4096 )

#creating cgroup
if [ -d  "/var/cgroups/cache-test-arghya" ]
then
	cgdelete memory:cache-test-arghya
fi
cgcreate -g "memory:cache-test-arghya" -t arghya:arghya
bash -c "echo 1 > /var/cgroups/cache-test-arghya/memory.oom_control"
bash -c "echo 1073741824 > /var/cgroups/cache-test-arghya/memory.limit_in_bytes"

NUMRUNS=1 #declaring number of runs
for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data_size[@]}-1; index++ ));
	do
		data_size_run=${data_size[$index]}
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		cgexec -g memory:cache-test-arghya ./executables/test $data_size_run
	done
done