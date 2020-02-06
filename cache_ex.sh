#!/bin/bash
#how-to-tun: sudo ./cache-ex.sh 32 128 cache-test-arghya
g++ ./merge-sort/binary-merge-sort.cpp -o ./executables/binary-merge-sort
chmod a+x ./executables/binary-merge-sort
g++ ./merge-sort/k-way-merge-sort.cpp -o ./executables/k-way-merge-sort
chmod a+x ./executables/k-way-merge-sort
g++ ./merge-sort/k-way-merge-sort-constant-memory.cpp -o ./executables/k-way-merge-sort-constant-memory
chmod a+x ./executables/k-way-merge-sort-constant-memory
g++ ./merge-sort/k-way-merge-sort-worst-case-memory.cpp -o ./executables/k-way-merge-sort-worst-case-memory
chmod a+x ./executables/k-way-merge-sort-worst-case-memory
g++ ./merge-sort/opt-extmem-merge-sort-const-mem.cpp -o ./executables/opt-extmem-merge-sort-const-mem
chmod a+x ./executables/opt-extmem-merge-sort-const-mem
g++ ./funnel-sort/funnel_sort.cpp -o ./executables/funnel_sort
chmod a+x ./executables/funnel_sort
g++ ./funnel-sort/funnel-sort-int.cpp -o ./executables/funnel-sort-int
chmod a+x ./executables/funnel-sort-int
g++ ./matrix-mul/cache_adaptive.cpp -o ./executables/cache-adaptive
chmod a+x ./executables/cache-adaptive
g++ ./matrix-mul/non_cache_adaptive.cpp -o ./executables/non-cache-adaptive
chmod a+x ./executables/non-cache-adaptive

if [ -d  "/var/cgroups/$1" ]
then
	cgdelete memory:$1
fi

cgcreate -g "memory:$1" -t arghya:arghya

if [ ! -f "/home/arghya/EM-MergeSort/merge-sort/nullbytes" ]
then
  echo "First creating file for storing data."
  dd if=/dev/urandom of=nullbytes count=16384 bs=1048576
fi

g++ ./large-file-creation/make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data
#./executables/make-unsorted-data $2
#sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
#sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

#cgexec -g memory:$3 ./executables/opt-extmem-merge-sort-const-mem $1 $2 $3

#code for constant memory profile
./executables/make-unsorted-data $2
sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

cgexec -g memory:$3 ./executables/k-way-merge-sort-constant-memory $1 $2 $3


./executables/make-unsorted-data $2
sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

cgexec -g memory:$3 ./executables/funnel-sort-int $1 $2 $3

#code for worst case memory profile

#./executables/make-unsorted-data $2
#sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
#sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

#cgexec -g memory:$3 ./executables/funnel_sort $1 $2 $3
#cgexec -g memory:$3 ./executables/k-way-merge-sort-worst-case-memory $1 $2 $3

#./executables/make-unsorted-data $2
#sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
#sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

#IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat /home/arghya/EM-MergeSort/mem_profile.txt))'
#cgexec -g memory:$3 ./executables/funnel-sort-int $1 $2 $3 &
#PID=$!
#STATUS=$(ps ax|grep "$PID"|wc -l)
#X=0
#CURRENT=0
#while [ $STATUS -ge 1 ] && [ $X -lt "${#XYZ[@]}" ]; do
#         VAL=${XYZ[$X]}
#   sleep $(calc "($VAL - $CURRENT) /1000")
#   CURRENT=$VAL
#         #bash -c "sleep $DIFF"
#   let X+=1
#   #echo 10000000 > /var/cgroups/$3/memory.limit_in_bytes
#   echo $((XYZ[X])) > /var/cgroups/$3/memory.limit_in_bytes
#   let X+=1
#   STATUS=$(ps ax|grep "$PID"|wc -l)
#done 
#echo "Success for worst case profile of funnel sort!"


#code for random memory profile
./executables/make-unsorted-data $2
sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"

cgexec -g memory:$3 ./executables/funnel-sort-int $1 $2 $3 &
PID=$!
STATUS=$(ps ax|grep "$PID"|wc -l)
CURRENT=$1 * 1024 * 1024
while [ $STATUS -ge 1 ] ; do
	sleep 5
	if (( $RANDOM % 2 == 1 )) || (( $CURRENT == $1 * 1024 * 1024 ));
	then
		echo "increasing memory"
		let CURRENT = CURRENT + $1 * 1024 * 1024
	else
		echo "decreasing memory"
		let CURRENT = CURRENT - $1 * 1024 * 1024
	fi
	echo $CURRENT > /var/cgroups/$1/memory.limit_in_bytes
	STATUS=$(ps ax|grep "$PID"|wc -l) 
done
echo "Success for random memory profile for funnel sort!"

