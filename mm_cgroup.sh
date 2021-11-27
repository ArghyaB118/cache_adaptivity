#!/bin/bash
set -ex
. ./configure.sh

now=$(date)
touch out-sorting.txt && echo "Running sorting adaptive memory experiment: $now" >> out-sorting.txt 
./cgroup_creation.sh cache-test-arghya $userid


for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data[@]}-1; index++ ));
	do
		input_data=${data[$index]}
		program_memory=${memory[$index]}
		fanout=8
		block_size=8192

		mem_profile="mem_profiles/sorting_profile_adversarial_$input_data.txt"
		[ -e $mem_profile ] && rm $mem_profile
		touch $mem_profile

		echo "Running EMS on worst-case memory"
		./executables/make-unsorted-data $input_data data_files/nullbytes
		drop_caches
		cgroup_limit_memory cache-test-arghya $program_memory
		cgexec -g memory:cache-test-arghya ./executables/EMS_cgroup adversarial $input_data $program_memory $fanout $block_size cache-test-arghya


		echo "Running EMS on worst-case memory"
		./executables/make-unsorted-data $input_data data_files/nullbytes
		drop_caches
		cgroup_limit_memory cache-test-arghya $program_memory
		IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat $mem_profile))'
		cgexec -g memory:cache-test-arghya ./executables/EMS_cgroup constant $input_data $program_memory $fanout $block_size cache-test-arghya &
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
		
		echo "Running LFS on worst-case memory"
		./executables/make-unsorted-data $input_data data_files/nullbytes
		drop_caches
		cgroup_limit_memory cache-test-arghya $program_memory
		IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat $mem_profile))'
		cgexec -g memory:cache-test-arghya ./executables/LFS $input_data data_files/nullbytes &
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
	done
done
