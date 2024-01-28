#!/bin/bash
set -ex
g++ -std=c++11 lcs-classic.cpp -o lcs_classic
g++ -std=c++11 lcs-hirschberg.cpp -o lcs_hirschberg
g++ -std=c++11 lcs-oblivious.cpp -o lcs_oblivious

declare -a length=( 524288 ) #( 65536 131072 262144 524288 )
declare -a programs=( "lcs_hirschberg" "lcs_oblivious" )

for l in "${length[@]}" ; do
	for p in "${programs[@]}" ; do
		filename="data/data-"$l".in"
		echo $filename $p $l
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename #dummy run
		echo "Running 1 concurrent process"
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		echo "Running 2 concurrent processes"
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
		echo "Running 4 concurrent processes"
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename &
		cgexec -g memory:cache-test-arghya ./$p $l 1 < $filename
		wait
	done
done