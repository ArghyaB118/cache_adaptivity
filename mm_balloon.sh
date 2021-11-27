#!/bin/bash
#It uses the balloons and the wall-time for the memory profiles
set -ex
. ./configure.sh

now=$(date)
touch out-mm.txt && echo "Running mm adaptive memory experiment: $now" >> out-mm.txt 
./cgroup_creation.sh cache-test-arghya $userid

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#matrixwidth[@]}-1; index++ ));
	do
	  	MATRIXWIDTH=${matrixwidth[$index]}
		STARTINGMEMORY_MB=${startingmemory[$index]}
		
		if $constant ; then
			touch dummy.txt

			echo "Running MM-INPLACE on constant memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 0 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j dummy.txt &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_scan $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes
			kill_balloons
			wait

			echo "Running MM-SCAN on constant memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 0 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j dummy.txt &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_inplace $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes
			kill_balloons
			wait
		fi

		if $adversarial ; then
			# setting the name of memory profile
			mem_profile="mem_profiles/mm_profile_adversarial_$MATRIXWIDTH.txt"
			[ -e $mem_profile ] && rm $mem_profile
			touch $mem_profile


			echo "Running MM-SCAN to generate worst-case memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 3 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_scan $MATRIXWIDTH $STARTINGMEMORY_MB adversarial data_files/nullbytes
			kill_balloons
			wait

			echo "Running MM-SCAN to replay worst-case memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_scan $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes
			kill_balloons
			wait

			echo "Running MM-INPLACE on worst-case memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_inplace $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes
			kill_balloons
			wait
		fi

		if $benevolent ; then
			# setting the name of memory profile
			mem_profile="mem_profiles/mm_profile_benevolent_$MATRIXWIDTH.txt"
			[ -e $mem_profile ] && rm $mem_profile
			touch $mem_profile


			echo "Running MM-SCAN to generate worst-case memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 3 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_scan $MATRIXWIDTH $STARTINGMEMORY_MB benevolent data_files/nullbytes
			kill_balloons
			wait

			echo "Running MM-SCAN to replay worst-case memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_scan $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes
			kill_balloons
			wait

			echo "Running MM-INPLACE on worst-case memory"
			./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes
			drop_caches
			cgroup_limit_memory cache-test-arghya $TOTALMEMORY
			reset_balloons $NUMBALLOONS
			for j in `seq 1 $NUMBALLOONS`;
			do
			     cgexec -g memory:cache-test-arghya ./executables/balloon > "balloon_data/balloon_log$j.txt" 1 $TOTALMEMORY_MB $STARTINGMEMORY_MB $NUMBALLOONS $j $mem_profile &
			done
			cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_inplace $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes
			kill_balloons
			wait
		fi

		if $oblivious ; then
			declare -a NUMINSTANCES=( 1 2 3 4 )
			for NUMINSTANCE in "${NUMINSTANCES[@]}" ; do
				for j in `seq 1 $NUMINSTANCE`;
				do
					[ ! -f "data_files/nullbytes$j" ] && dd if=/dev/urandom of=data_files/nullbytes$j count=32768 bs=1048576
					./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes$j
				done
				drop_caches
				echo "MM-SCAN on two 2 $MATRIXWIDTH x $MATRIXWIDTH matrices and memory size $STARTINGMEMORY_MB for oblivious memory with $NUMINSTANCE uniform program instances" >> out-mm.txt 
				cgroup_limit_memory cache-test-arghya $TOTALMEMORY
				for j in `seq 1 $NUMINSTANCE`;
				do
					cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_scan $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes$j &
				done
				wait

				for j in `seq 1 $NUMINSTANCE`;
				do
					./executables/make-mm-data $MATRIXWIDTH data_files/nullbytes$j
				done
				drop_caches
				echo "MM-INPLACE on two $MATRIXWIDTH x $MATRIXWIDTH matrices and memory size $STARTINGMEMORY_MB for oblivious memory with $NUMINSTANCE uniform program instances" >> out-mm.txt 
				cgroup_limit_memory cache-test-arghya $TOTALMEMORY
				for j in `seq 1 $NUMINSTANCE`;
				do
					cgexec -g memory:cache-test-arghya ./executables/mm_balloon mm_inplace $MATRIXWIDTH $STARTINGMEMORY_MB constant data_files/nullbytes$j &
				done
				wait
			done
		fi
  	done
done
