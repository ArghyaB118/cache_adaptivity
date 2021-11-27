#!/bin/bash
set -x

if [ $# -ne 2 ]
then
	echo "Must supply 2 arguments. \n \
	Usage: sudo ./cgroups.sh <cgroup_name> <user_id>"
fi

if [ -d  "/var/cgroups/$1" ]
then
	cgdelete memory:$1
fi

cgcreate -g "memory:$1" -t $2:$2
if [ -d "/var/cgroups/$1" ]
then
	bash -c "echo 1 > /var/cgroups/$1/memory.oom_control"
fi

if [ -d "/sys/fs/cgroup/memory/$1" ]
then
sudo bash -c "echo 1 > /sys/fs/cgroup/memory/$1/memory.oom_control"
fi
echo "cgroup is created successfully"