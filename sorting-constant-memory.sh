#!/bin/bash
set -ex
. ./configure.sh

now=$(date)
touch out-sorting.txt && echo "Running sorting constant memory experiment: $now" >> out-sorting.txt 
./cgroup_creation.sh cache-test-arghya $userid

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data_size[@]}-1; index++ ));
	do
		data_size_run=${data_size[$index]}
		memory_given_run=${memory_given[$index]}
		memory_given_bytes=$((memory_given_run*1024*1024))
		block_size=8192

		if $constant_memory ; then
			if $LFS ; then
				#code for constant memory profile funnel sort
				./executables/make-unsorted-data $data_size_run $datafile
				drop_caches
				echo "funnel sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
				cgroup_limit_memory cache-test-arghya $memory_given_bytes
				cgexec -g memory:cache-test-arghya ./executables/LFS $data_size_run $datafile
			fi
			if $EMS ; then
			for (( fanout=0; fanout<=${#fanouts[@]}-1; fanout++ ));
				do
					#code for constant memory profile merge sort with variable fanout
					./executables/make-unsorted-data $data_size_run $datafile
					drop_caches
					echo "merge sort constant memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
					cgroup_limit_memory cache-test-arghya $memory_given_bytes
					cgexec -g memory:cache-test-arghya ./executables/EMS $data_size_run ${fanouts[$fanout]} $block_size $datafile
				done
			fi
		fi
		if $double_memory ; then
			if $LFS ; then
				#code for double memory profile funnel sort
				./executables/make-unsorted-data $data_size_run $datafile
				drop_caches
				echo "funnel sort double memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
				cgroup_limit_memory cache-test-arghya $((2*memory_given_bytes))
				cgexec -g memory:cache-test-arghya ./executables/LFS $data_size_run $datafile
			fi
			if $EMS ; then
				for (( fanout=0; fanout<=${#fanouts[@]}-1; fanout++ ));
				do
					#code for double memory profile merge sort with variable fanout
					./executables/make-unsorted-data $data_size_run $datafile
					drop_caches
					echo "merge sort double memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
					cgroup_limit_memory cache-test-arghya $((2*memory_given_bytes))
					cgexec -g memory:cache-test-arghya ./executables/EMS $data_size_run ${fanouts[$fanout]} $block_size $datafile
				done
			fi
		fi
		if $half_memory ; then
			if $LFS ; then
				#code for half memory profile funnel sort
				./executables/make-unsorted-data $data_size_run $datafile
				drop_caches
				echo "funnel sort half memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
				cgroup_limit_memory cache-test-arghya $((memory_given_bytes/2))
				cgexec -g memory:cache-test-arghya ./executables/LFS $data_size_run $datafile
			fi
			if $EMS ; then
				for (( fanout=0; fanout<=${#fanouts[@]}-1; fanout++ ));
				do
					#code for half memory profile merge sort with variable fanout
					./executables/make-unsorted-data $data_size_run $datafile
					drop_caches
					echo "merge sort half memory for data size $data_size_run and memory size $memory_given_run" >> out-sorting.txt 
					cgroup_limit_memory cache-test-arghya $((memory_given_bytes/2))
					cgexec -g memory:cache-test-arghya ./executables/EMS $data_size_run ${fanouts[$fanout]} $block_size $datafile
				done
			fi
		fi
	done
done
