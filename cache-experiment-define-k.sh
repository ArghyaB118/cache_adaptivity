#!/bin/bash
g++ ./merge-sort/opt-extmem-merge-sort.cpp -o ./executables/opt-extmem-merge-sort
chmod a+x ./executables/opt-extmem-merge-sort
g++ ./funnel-sort/funnel-sort-int.cpp -o ./executables/funnel-sort-int
chmod a+x ./executables/funnel-sort-int
g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data

# making the nullbytes
if [ ! -f "merge-sort/nullbytes" ]
then
  echo "First creating file for storing data."
  dd if=/dev/urandom of=nullbytes count=32768 bs=1048576
  mv nullbytes merge-sort/nullbytes
fi
# deleting out-sorting.txt and creating again
if [ -f "out-sorting.txt" ]
then
  echo "out-sorting.txt already exists. Deleting it first."
  rm out-sorting.txt
  touch out-sorting.txt
fi

# deleting mem_profile.txt and creating again
if [ -f "mem_profile.txt" ]
then
  echo "mem_profile.txt already exists. Deleting it first."
  rm mem_profile.txt
  touch mem_profile.txt
fi

# to achieve depth 4 while using 4-way merge sort
declare -a data_size=( 8192 )
declare -a memory_given=( 256 ) # here M/4B is 1024
NUMRUNS=1

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data_size[@]}-1; index++ ));
	do
		data_size_run=${data_size[$index]}
		memory_given_run=${memory_given[$index]}
		#code for constant memory profile merge sort
#		./cgroup_creation.sh cache-test-arghya
#		./executables/make-unsorted-data $data_size_run
#		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
#		echo "merge sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
#		cgexec -g memory:cache-test-arghya ./executables/opt-extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya 1 4 16 #3
		#./executables/opt-extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya 1 4 8

		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo "merge sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
		cgexec -g memory:cache-test-arghya ./executables/opt-extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya 1 8 16 #2


		#./cgroup_creation.sh cache-test-arghya
		#./executables/make-unsorted-data $data_size_run
		#sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		#echo "merge sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
		#cgexec -g memory:cache-test-arghya ./executables/funnel-sort-int $memory_given_run $data_size_run cache-test-arghya
	done
done
