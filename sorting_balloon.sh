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
		program_memory=${memory[$index]} #program memory in bytes
		total_memory=$((input_data+program_memory)) #total memory for cgroup in bytes
		fanout=${fanouts[$index]}
		block_size=8192

		if $constant ; then
			touch dummy.txt

			echo "Running EMS on constant memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 0 $total_memory $program_memory $NUMBALLOONS $j dummy.txt &
			done
			cgexec -g memory:cache-test-arghya ./executables/EMS_balloon constant $input_data $program_memory $fanout $block_size
			kill_balloons
			wait
				
			echo "Running LFS on constant memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 0 $total_memory $program_memory $NUMBALLOONS $j dummy.txt &
			done
			cgexec -g memory:cache-test-arghya ./executables/LFS $input_data data_files/nullbytes
			kill_balloons
			wait
		fi


		if $adversarial ; then
			# setting the name of memory profile
			mem_profile="mem_profiles/sorting_profile_adversarial_$input_data.txt"
			[ -e $mem_profile ] && rm $mem_profile
			touch $mem_profile

			echo "Running merge sort to generate worst-case memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 3 $total_memory $program_memory $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/EMS_balloon adversarial $input_data $program_memory $fanout $block_size
			kill_balloons
			wait
			
			echo "Running merge sort to replay worst-case memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $total_memory $program_memory $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/EMS_balloon constant $input_data $program_memory $fanout $block_size
			kill_balloons
			wait
			
			echo "Running funnel sort on worst-case memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $total_memory $program_memory $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/LFS $input_data data_files/nullbytes
			kill_balloons
			wait
		fi



		if $benevolent ; then
			# setting the name of memory profile
			mem_profile="mem_profiles/sorting_profile_benevolent_$input_data.txt"
			[ -e $mem_profile ] && rm $mem_profile
			touch $mem_profile

			echo "Running merge sort to generate benevolent memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 3 $total_memory $program_memory $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/EMS_balloon benevolent $input_data $program_memory $fanout $block_size
			kill_balloons
			wait
			
			echo "Running merge sort to replay benevolent memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $total_memory $program_memory $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/EMS_balloon constant $input_data $program_memory $fanout $block_size
			kill_balloons
			wait
			
			echo "Running funnel sort on benevolent memory"
			./executables/make-unsorted-data $input_data data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $((total_memory*1024*1024))
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $total_memory $program_memory $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/LFS $input_data data_files/nullbytes
			kill_balloons
			wait
		fi

		if $oblivious ; then
			declare -a NUMINSTANCES=( 1 2 3 4 )
			for NUMINSTANCE in "${NUMINSTANCES[@]}" ; do
				for j in `seq 1 $NUMINSTANCE`;
				do
					[ ! -f "data_files/nullbytes$j" ] && dd if=/dev/urandom of=data_files/nullbytes$j count=32768 bs=1048576
					./executables/make-unsorted-data $input_data data_files/nullbytes$j
				done
				drop_caches
				echo "EMS for data size $input_data and memory size $program_memory for oblivious memory with $NUMINSTANCE uniform program instances" >> out-sorting.txt 
				cgroup_limit_memory cache-test-arghya $((program_memory*1024*1024))
				for j in `seq 1 $NUMINSTANCE`;
				do
					cgexec -g memory:cache-test-arghya ./executables/EMS $input_data $fanout $block_size data_files/nullbytes$j &
				done
				wait

				for j in `seq 1 $NUMINSTANCE`;
				do
					./executables/make-unsorted-data $input_data data_files/nullbytes$j
				done
				drop_caches
				echo "LFS for data size $input_data and memory size $program_memory for oblivious memory with $NUMINSTANCE uniform program instances" >> out-sorting.txt 
				cgroup_limit_memory cache-test-arghya $((program_memory*1024*1024))
				for j in `seq 1 $NUMINSTANCE`;
				do
					cgexec -g memory:cache-test-arghya ./executables/LFS $input_data data_files/nullbytes$j &
				done
				wait
			done
		fi

		if $database ; then
			cgexec -g memory:cache-test-arghya mysql -u root -parghya118 -e "CREATE DATABASE tpcc1000;"
			cgexec -g memory:cache-test-arghya mysql -u root -parghya118 tpcc1000 < create_table.sql > ~/tpcc-output-ps-55-bpool-256.log
			cgexec -g memory:cache-test-arghya mysql -u root -parghya118 tpcc1000 < add_fkey_idx.sql > ~/tpcc-output-ps-55-bpool-256.log
			cgexec -g memory:cache-test-arghya ./tpcc_load -h127.0.0.1 -dtpcc1000 -uroot -parghya118 -w20 > ~/tpcc-output-ps-55-bpool-256.log
			for i in {1..100}
			do
				cgexec -g memory:cache-test-arghya ./tpcc_start -h127.0.0.1 -dtpcc1000 -uroot -parghya118 -w20 -c16 -r10 -l1200 > ~/tpcc-output-ps-55-bpool-256.log
			done
		fi
	done
done
