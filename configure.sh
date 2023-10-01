#!/bin/bash
set -ex
[ ! -d "executables" ] && mkdir executables

g++ ./large-file-creation/make-mm-data.cpp -o ./executables/make-mm-data
chmod a+x ./executables/make-mm-data
g++ ./balloon.cpp -o ./executables/balloon
chmod a+x ./executables/balloon
g++ ./matrix-mul/mm_balloon.cpp -o ./executables/mm_balloon
chmod a+x ./executables/mm_balloon
g++ ./merge-sort/EMS_balloon.cpp -o ./executables/EMS_balloon
chmod a+x ./executables/EMS_balloon
g++ ./merge-sort/EMS_optimized.cpp -o ./executables/EMS
chmod a+x ./executables/EMS
g++ ./funnel-sort/LFS.cpp -o ./executables/LFS
chmod a+x ./executables/LFS
g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data
g++ ./merge-sort/EMS_cgroup.cpp -o ./executables/EMS_cgroup
chmod a+x ./executables/EMS_cgroup
chmod +x setup_stxxl.sh

if [ ! -d "data_files" ]
then
	mkdir data_files
fi
[ ! -d "balloon_data" ] && mkdir balloon_data
# create folder mem_profiles
if [ ! -d "mem_profiles" ]
then
  mkdir mem_profiles
fi


# making the nullbytes
if [ ! -f "data_files/nullbytes" ]
then
	echo "First creating file for storing data."
	dd if=/dev/urandom of=data_files/nullbytes count=32768 bs=1048576
fi
if [ ! -f "balloon_data/IPCTEST" ]
then
  echo "First creating file for IPCTEST."
  dd if=/dev/urandom of=balloon_data/IPCTEST count=8 bs=1048576
fi

userid="arghya"
touch out-mm.txt && touch out-sorting.txt

#########################################
# configuarion parameters for the mm tests
declare -a matrixwidth=( 1000 2000 3000 4000 5000 6000 )
declare -a startingmemory=( 10 10 10 10 10 10 )
NUMBALLOONS=9 #used for mm
TOTALMEMORY_MB=100
TOTALMEMORY=$((TOTALMEMORY_MB*1024*1024))
NUMRUNS=1

constant=true
adversarial=false
benevolent=false
oblivious=true
database=true

constant_memory=true
double_memory=false
half_memory=false
EMS=true
LFS=false

# configuarion parameters for sorting experiments
declare -a data=( 512 ) #input data size given in MiB
declare -a memory=( 64 ) #input memory size given in MiB
declare -a fanouts=( 8 8 8 8 8 8 8 ) #this is used in the balloon program
declare -a fanouts_varied=( 32 ) #this is used in the constant memory program
block_size=2048 #this is used in both balloon and constant memory program

NUMBALLOONS=2 #used for sorting
datafile="data_files/nullbytes"
#########################################



function drop_caches() {
    for i in {0..4}
    do
		sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		sleep 3
    done
}

# Call cgroup_limit_memory <cgroup_name> <memory>
function cgroup_limit_memory() {
	if [ -d "/var/cgroups/$1" ]
	then
		bash -c "echo $2 > /var/cgroups/$1/memory.limit_in_bytes"
	fi
	if [ -d "/sys/fs/cgroup/memory/$1" ]
	then
		bash -c "echo $2 > /sys/fs/cgroup/memory/$1/memory.limit_in_bytes"
	fi
}

# Each balloon can take at most 1GiB space
# Call reset_balloons <$NUMBALLOONS>
function reset_balloons() {
	for j in `seq 1 $1`;
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
}


function kill_balloons() {
	pids=( $(pgrep -f balloon) )
	for pid in "${pids[@]}"; do
		if [[ $pid != $$ ]]; then
			kill "$pid"
		fi
	done
	sleep 3
}
