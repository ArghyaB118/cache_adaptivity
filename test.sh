#!/bin/bash
set -ex
g++ ./merge-sort/extmem-merge-sort-balloon.cpp -o ./executables/extmem-merge-sort-balloon
chmod a+x ./executables/extmem-merge-sort-balloon
g++ ./funnel-sort/lazy-funnel-sort-balloon.cpp -o ./executables/lazy-funnel-sort-balloon
chmod a+x ./executables/lazy-funnel-sort-balloon
g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data
g++ ./balloon.cpp -o ./executables/balloon
chmod a+x ./executables/balloon

NUMRUNS=1
NUMBALLOONS=11

#declare -a data_size=( 512 1024 2048 4096 8192 ) 
#declare -a memory_given=( 256 256 256 256 256 )
declare -a data_size=( 64 ) 
declare -a memory_given=( 32 )

#creating nullbytes
if [ ! -f "merge-sort/nullbytes" ]
then
  echo "First creating file for storing data."
  dd if=/dev/urandom of=merge-sort/nullbytes count=32768 bs=1048576
fi
if [ ! -f "balloon_data/IPCTEST" ]
then
  echo "First creating file for IPCTEST."
  dd if=/dev/urandom of=balloon_data/IPCTEST count=32768 bs=32
fi
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

		

		#code to generate random memory profile from merge sort
		echo "Running merge sort to generate worst case memory"
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		sudo bash -c "echo 1 > /var/cgroups/cache-test-arghya/memory.oom_control"
		echo "merge sort to generate worse case memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt
		echo $TOTALMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		for j in `seq 1 $NUMBALLOONS`;
		do
		     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 3 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j &
		done
		cgexec -g memory:cache-test-arghya ./executables/extmem-merge-sort-balloon $memory_given_run $data_size_run cache-test-arghya 3 8 1024
		pids=( $(pgrep -f balloon) )
		for pid in "${pids[@]}"; do
		if [[ $pid != $$ ]]; then
		 kill "$pid"
		fi
		done
		sleep 5
		wait
		# #putting the important memory profile in a separate file before the two next sections
		cp mem_profile.txt mem_profile_use.txt
		for j in `seq 1 $NUMBALLOONS`;
		do
		     rm balloon_data/balloon_log$j.txt
		done

		#code to replay random memory profile from merge sort
		echo "Running merge sort to replay worst case memory"
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		sudo bash -c "echo 1 > /var/cgroups/cache-test-arghya/memory.oom_control"
		echo "merge sort to replay worse case memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt
		echo $TOTALMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		for j in `seq 1 $NUMBALLOONS`;
		do
		     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j &
		done
		cgexec -g memory:cache-test-arghya ./executables/extmem-merge-sort-balloon $memory_given_run $data_size_run cache-test-arghya 1 8 1024
		pids=( $(pgrep -f balloon) )
		for pid in "${pids[@]}"; do
		if [[ $pid != $$ ]]; then
		 kill "$pid"
		fi
		done
		sleep 5
		wait
		for j in `seq 1 $NUMBALLOONS`;
		do
		     rm balloon_data/balloon_log$j.txt
		done

		#run cache-adaptive on random memory
		echo "Running funnel sort on worst-case memory"
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		sudo bash -c "echo 1 > /var/cgroups/cache-test-arghya/memory.oom_control"
		echo "funnel sort on worse case memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt
		echo $TOTALMEMORY > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		for j in `seq 1 $NUMBALLOONS`;
		do
		     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j &
		done
		cgexec -g memory:cache-test-arghya ./executables/lazy-funnel-sort-balloon $memory_given_run $data_size_run cache-test-arghya
		pids=( $(pgrep -f balloon) )
		for pid in "${pids[@]}"; do
		if [[ $pid != $$ ]]; then
		 kill "$pid"
		fi
		done
		sleep 5
	done
done
