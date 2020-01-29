#include<iostream>
//#include <bits/stdc++.h>
using namespace std;

int main() {
	// No. of Partitions of input file. 
    int num_ways = 10; 
    // The size of each partition 
    int run_size = 1000; 
	char input_file[] = "input.txt"; 
    FILE* in = fopen(input_file, "w");
    srand(time(NULL)); 
  
    // generate input 
    for (int i = 0; i < num_ways * run_size; i++) 
        fprintf(in, "%d\n", rand()); 
  
    fclose(in); 
}