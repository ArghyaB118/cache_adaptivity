#!/bin/bash
set -ex
g++ ./merge-sort/extmem-merge-sort-balloon.cpp -o ./executables/extmem-merge-sort-balloon
chmod a+x ./executables/extmem-merge-sort-balloon
g++ ./funnel-sort/funnel-sort-int.cpp -o ./executables/funnel-sort-int
chmod a+x ./executables/funnel-sort-int
g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data
g++ ./balloon.cpp -o ./executables/balloon
chmod a+x ./executables/balloon

NUMRUNS=1
NUMBALLOONS=3

#declare -a data_size=( 512 1024 1536 2048 2560 3072 3584 4096 5120 6144 7168 8192 ) 
#declare -a memory_given=( 256 256 256 256 256 256 256 256 256 256 256 256 )
declare -a data_size=( 512 ) 
declare -a memory_given=( 256 )

#creating nullbytes
if [ ! -f "merge-sort/nullbytes" ]
then
  echo "First creating file for storing data."
  dd if=/dev/urandom of=nullbytes count=32768 bs=1048576
  mv nullbytes merge-sort/nullbytes
fi
#deleting out-sorting.txt and creating again
if [ -f "out-sorting.txt" ]
then
  echo "out-sorting.txt already exists. Deleting it first."
  rm out-sorting.txt
  touch out-sorting.txt
fi

for j in `seq 1 $NUMBALLOONS`;
do
	if [ -f "balloon_data/balloon_data$j" ]
	then
		rm -r balloon_data/balloon_data$j
	fi
	dd if=/dev/urandom of=balloon_data/balloon_data$j count=1024 bs=1048576
	if [ -f "balloon_data/balloon_log$j.txt" ]
	then
  		rm balloon_data/balloon_log$j.txt
	fi
done

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data_size[@]}-1; index++ ));
	do
		data_size_run=${data_size[$index]}
		memory_given_run=${memory_given[$index]}
		STARTINGMEMORY_MB=$memory_given_run
		TOTALMEMORY_MB=$((data_size_run+memory_given_run))
		TOTALMEMORY=$(($TOTALMEMORY_MB*1024*1024))

		#code to generate worse case memory profile from merge sort
		echo "Running merge sort to generate worst case memory"
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		sudo bash -c "echo 1 > /var/cgroups/cache-test-arghya/memory.oom_control"
		echo "merge sort to generate worse case memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt
		echo $TOTALMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		for j in `seq 1 $NUMBALLOONS`;
		do
			cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j &
		done
		sleep 150
		pkill -f balloon --signal SIGTERM
		wait
	done
done