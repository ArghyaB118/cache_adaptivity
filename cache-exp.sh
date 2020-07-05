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
declare -a data_size=( 512 )
declare -a memory_given=( 256 ) # here M/4B is 1024
NUMRUNS=3

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data_size[@]}-1; index++ ));
	do
		data_size_run=${data_size[$index]}
		memory_given_run=${memory_given[$index]}
		
		#code to generate worse case memory profile from merge sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		sudo bash -c "echo 1 > /var/cgroups/cache-test-arghya/memory.oom_control"
		echo "merge sort to generate worse case memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt
		cgexec -g memory:cache-test-arghya ./executables/opt-extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya 2 4 8


		#code to run worse case memory profile on merge sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		#putting the important memory profile in a separate file before the two next sections
		cp mem_profile.txt mem_profile_use.txt
		IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat mem_profile_use.txt))'
		echo "merge sort rerun on worse case memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt
		cgexec -g memory:cache-test-arghya ./executables/opt-extmem-merge-sort $memory_given_run $data_size_run cache-test-arghya 1 4 8 &
		PID=$!
		STATUS=$(ps ax|grep "$PID"|wc -l)
		X=0
		CURRENT=0
		while [ $STATUS -gt 1 ] && [ $X -lt "${#XYZ[@]}" ]; do
		   VAL=${XYZ[$X]%.*}
		   sleep $(calc "($VAL-$CURRENT)")
		   CURRENT=$VAL
		   let X+=1
		   echo "sleeping $CURRENT seconds and writing memory size $((XYZ[X]))"
		   echo $((XYZ[X])) > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		   let X+=1
		   STATUS=$(ps ax|grep "$PID"|wc -l)
		done 
		echo "Success for worse case profile on merge sort!"
		wait

		#code to run worse case memory profile on funnel sort
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat mem_profile_use.txt))'
		echo "funnel sort on worse case memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt
		cgexec -g memory:cache-test-arghya ./executables/funnel-sort-int $memory_given_run $data_size_run cache-test-arghya &
		PID=$!
		STATUS=$(ps ax|grep "$PID"|wc -l)
		X=0
		CURRENT=0
		while [ $STATUS -gt 1 ] && [ $X -lt "${#XYZ[@]}" ]; do
		   VAL=${XYZ[$X]%.*}
		   sleep $(calc "($VAL-$CURRENT)/5")
		   CURRENT=$VAL
		   let X+=1
		   echo "sleeping $CURRENT seconds and writing memory size $((XYZ[X]))"
		   echo $((XYZ[X])) > /var/cgroups/cache-test-arghya/memory.limit_in_bytes
		   let X+=1
		   STATUS=$(ps ax|grep "$PID"|wc -l)
		done 
		echo "Success for worse case profile on funnel sort!"
		wait

	done
done