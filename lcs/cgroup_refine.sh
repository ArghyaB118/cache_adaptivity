#!/bin/bash
set -ex
cgdelete memory:cache-test-arghya
cgcreate -g memory:cache-test-arghya -t arghya:arghya
cat /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
bash -c "echo 4194304 > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes"
cat /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
cat /sys/fs/cgroup/memory/cache-test-arghya/memory.oom_control
bash -c "echo 1 > /sys/fs/cgroup/memory/cache-test-arghya/memory.oom_control"
cat /sys/fs/cgroup/memory/cache-test-arghya/memory.oom_control
