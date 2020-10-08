#!/bin/bash
set -ex
g++ ./merge-sort/extmem-merge-sort.cpp -o ./executables/extmem-merge-sort
chmod a+x ./executables/extmem-merge-sort
g++ ./funnel-sort/lazy-funnel-sort.cpp -o ./executables/lazy-funnel-sort
chmod a+x ./executables/lazy-funnel-sort
g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data

NUMRUNS=1
NUMINSTANCE=2

declare -a data_size=( 256 ) 
declare -a memory_given=( 256 )

#creating nullbytes
for i in `seq 1 $NUMINSTANCE`;
do
	if [ ! -f "data_files/nullbytes$i" ]
	then
	  echo "First creating file for storing data."
	  dd if=/dev/urandom of=data_files/nullbytes$i count=32768 bs=1048576
	fi
done
#deleting out-sorting.txt and creating again
if [ -f "out-sorting.txt" ]
then
  echo "out-sorting.txt already exists. Deleting it first."
  rm out-sorting.txt
  touch out-sorting.txt
fi
if [ -f "log.txt" ]
then
  echo "log.txt already exists. Deleting it first."
  rm log.txt && touch out-sorting.txt
fi


if [ -d "memory_profiles" ]
then
  rm -r memory_profiles
fi
mkdir memory_profiles

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data_size[@]}-1; index++ ));
	do
		data_size_run=${data_size[$index]}
		memory_given_run=${memory_given[$index]}
		STARTINGMEMORY_MB=$memory_given_run
		STARTINGMEMORY=$(($STARTINGMEMORY_MB*1024*1024))
		fanout=8
		block_size=8192

		# #code for competition among merge sort
		# echo "Running merge sort on constant memory"
		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo "merge sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
		# echo $STARTINGMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya data_files/nullbytes1 $fanout $block_size 
		# sleep 5
		# wait

		#code for competition among funnel sort
		echo "Running funnel sort on constant memory"
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo "funnel sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
		echo $STARTINGMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/lazy-funnel-sort $memory_given_run $data_size_run data_files/nullbytes1
		sleep 5
		wait

		#code for competition among merge sort
		echo "Running merge sort with competition"
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		./executables/make-unsorted-data $data_size_run data_files/nullbytes2
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo "merge sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
		echo $STARTINGMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya data_files/nullbytes1 $fanout $block_size &
		cgexec -g memory:cache-test-arghya ./executables/extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya data_files/nullbytes2 $fanout $block_size 
		sleep 5
		wait

		# #code for competition among funnel sort
		# echo "Running funnel sort with competition"
		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes2
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo "funnel sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
		# echo $STARTINGMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/lazy-funnel-sort $memory_given_run $data_size_run cache-test-arghya data_files/nullbytes1 &
		# cgexec -g memory:cache-test-arghya ./executables/lazy-funnel-sort $memory_given_run $data_size_run cache-test-arghya data_files/nullbytes2
		# sleep 5
		# wait
	done
done
