#!/bin/bash
pids=( $(pgrep -f balloon) )
for pid in "${pids[@]}"; do
	if [[ $pid != $$ ]]; then
		kill "$pid"
	fi
done
sleep 3