#include <iostream>
#include <vector>
#include "time.h"

using namespace std;

int main () {
	vector<int> v = {};
	for (int i = 1; i < 1024 * 1024 * 1024; i++)
		v.push_back(i);
	time_t a = time(NULL);
	for (int i = 0; i < 10; i++) {
		for (auto & j : v)
			int k = j;
	}
	time_t b = time(NULL);
	cout << b - a << endl;
	return 0;
}

/*
Method to set memory:
sudo cgdelete memory:cache-test-arghya
sudo cgcreate -g memory:cache-test-arghya -t arghya:arghya
sudo sh -c "echo 4294967296 > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes"
Experiment:
Looping over 4 GiB inetger array 10 times
Result:
memory: 1 GiB => time = 235 s
memory: 2 GiB => time = 237 s
memory: 4 GiB => time = 243 s 
memory: 8 GiB => time = 69 s
memory 16 giB => time = 70 s
*/
