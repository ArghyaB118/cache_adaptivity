#! /bin/bash
now=$(date)
echo "$now"
if [ -d "/sys/fs/cgroup/memory/cache-test-arghya" ]
then
cgdelete memory:cache-test-arghya
fi
cgcreate -g memory:cache-test-arghya
bash -c "echo 1 > /sys/fs/cgroup/memory/cache-test-arghya/memory.oom_control"
bash -c "echo 4194304 > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes"
declare -a ip_size=( "8192" "16384" "32768" "65536" "131072" "262144" "524288" "1048576" "2097152" "4194304" "8388608" )
numruns=1

for (( j=0; j<=${#ip_size[@]}-1; j++ ));
do
cgexec -g memory:cache-test-arghya ./lcs-hirschberg-opt-uneq ${ip_size[$j]} $numruns < data/data-${ip_size[$j]}.in
cgexec -g memory:cache-test-arghya ./lcs-our-alg-opt-uneq ${ip_size[$j]} $numruns < data/data-${ip_size[$j]}.in
#cgexec -g memory:cache-test-arghya 
./lcs-classic ${ip_size[$j]} $numruns < data/data-${ip_size[$j]}.in 
done


