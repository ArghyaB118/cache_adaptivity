#!/bin/bash
set -x

IFS=$'\r\n ' GLOBIGNORE='*' command eval  'XYZ=($(cat /home/arghya/EM-MergeSort/merge-sort/mem_profile.txt))'

if [ $# -ne 3 ]
then
	echo "Must supply 3 arguments. \n \
	Usage: sudo ./cache.sh <program> <cgroup_memory_in_mebibytes> <cgroup_name> \n \
	The possible values for <program> are: \n \
	0 = test program \n \
	1 = cache_adaptive matrix multiply \n \
	2 = non_cache_adaptive matrix multiply \n"
	exit
fi


sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
sudo bash -c "echo 1 > /var/cgroups/$3/memory.oom_control"
if [ -d  "/var/cgroups/$3" ]
then
	cgdelete memory:$3
fi

cgcreate -g "memory:$3" -t arghya:arghya
cgexec -g memory:$3 ./executables/funnel_sort $1 $2 $3 > log1.txt &
PID=$! 
STATUS=$(ps ax|grep "$PID"|wc -l)
X=0
CURRENT=0
while [ $STATUS -ge 1 ] && [ $X -lt "${#XYZ[@]}" ]; do
	VAL=${XYZ[$X]}
  sleep $(calc "($VAL - $CURRENT) /1000")
  CURRENT=$VAL
	#bash -c "sleep $DIFF"
  let X+=1
  #echo 10000000 > /var/cgroups/$3/memory.limit_in_bytes
  echo $((XYZ[X])) > /var/cgroups/$3/memory.limit_in_bytes
  let X+=1
  STATUS=$(ps ax|grep "$PID"|wc -l)
done

echo "Success!"
