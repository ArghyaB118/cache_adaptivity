#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <vector>
#include <ctime>

using namespace std;
int main() {
	vector<int> random_memory;
	int total_memory = 2048, base_case = 16, k = 4;
	int i = total_memory, counter = 1;
	random_memory.push_back(i);
	while (i > base_case) {
		i = i / k;
		counter = counter * k;
		for (int j = 0; j < counter; ++j)
			random_memory.push_back(i);
	}
	for (auto j = random_memory.begin(); j != random_memory.end(); ++j) 
        cout << *j << " "; 
    srand((unsigned) time(0));
    int element =  rand() % random_memory.size();
    cout << endl << random_memory.size() << " " << element << " " << random_memory[element] << endl;
	return 0;
}