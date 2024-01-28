#!/bin/bash
set -ex
g++ -std=c++11 lcs-classic.cpp -o lcs_classic
g++ -std=c++11 lcs-hirschberg.cpp -o lcs_hirschberg
g++ -std=c++11 lcs-oblivious.cpp -o lcs_oblivious

# declare -a length=( 262144 524288 1048576 2097152 ) experiment2
declare -a length=( 524288 1048576 2097152 4194304 ) #experiment 2a
declare -a programs=( "lcs_hirschberg" "lcs_oblivious" )
memory=4194304

for l in "${length[@]}" ; do
	for p in "${programs[@]}" ; do
		filename="data/data-"$l".in"
		bash -c "echo $memory > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes"
		echo $filename $p $l $memory
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename #dummy run
		echo "Running 1 concurrent process"
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
		memory=$(($memory*2))
		bash -c "echo $memory > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes"
		echo "Running 2 concurrent processes"
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
		memory=$(($memory*2))
		bash -c "echo $memory > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes"
		echo "Running 4 concurrent processes"
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
		memory=$(($memory/2))
	done
done