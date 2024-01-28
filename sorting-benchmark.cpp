#include <iostream>
#include <fstream>
#include <vector>
#include <sys/mman.h>

#include <string>
#include <cstring>

#include <fcntl.h> // For O_RDWR
#include <unistd.h> // For open()

#include "sorting/ems.h"
#include "sorting/funnelSort.h"

using namespace std;

int main (int argc, char *argv[]) {
	if (argc < 5){
		std::cout << "Insufficient arguments!" << 
		"Usage: ./a.out <data_MiB> <memory_MiB> <logical_block_size_KiB> <datafile>" << endl;
		exit(1);
	}
	TYPE *arr;
	// handling command line inputs
	int data_MiB = atoi(argv[1]);
	TYPE num_elements = data_MiB * 1024 * 1024 / sizeof(TYPE);
	int memory_MiB = atoi(argv[2]);
	int logical_block_KiB = atoi(argv[3]);
	char* datafile = new char[strlen(argv[4]) + 1](); 
	strncpy(datafile, argv[4], strlen(argv[4]));

	/* M/B-way merge, to be precise M/2B */
	int fanout = memory_MiB * 1024 / logical_block_KiB / 2;
	TYPE base_case = logical_block_KiB * 1024 / sizeof(TYPE);
	TYPE memory_bytes = memory_MiB * 1024 * 1024;

	// opening input file
	int fdout;
	if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
		cout << "can't create nullbytes for writing" << endl;
		return 0;
	}
	// mmapping from input file
	if (((arr = (TYPE*) mmap (0, sizeof(TYPE) * num_elements, 
		PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) 
		== (TYPE*) MAP_FAILED)){
		cout << "mmap error for output with code" << endl;
		return 0;
	}

	Sorter s(num_elements, fanout, base_case, memory_bytes);
	s.makeUnsortedArray (arr, num_elements);
	cout << boolalpha << s.isSorted(arr, num_elements) << endl;

	double start_time = get_wall_time();
	std::sort (arr, arr + num_elements);
	double finish_time = get_wall_time();
	cout << "Time needed for STL sort: "
		<< finish_time - start_time << endl;

	cout << boolalpha << s.isSorted(arr, num_elements) << endl;
	s.makeUnsortedArray (arr, num_elements);
	cout << boolalpha << s.isSorted(arr, num_elements) << endl;

	start_time = get_wall_time();
	s.binaryMergeSort (arr, 0, num_elements);
	finish_time = get_wall_time();
	cout << "Time needed for binary merge sort: "
		<< finish_time - start_time << endl;

	// s.printArray (arr, num_elements);
	cout << boolalpha << s.isSorted(arr, num_elements) << endl;
	s.makeUnsortedArray (arr, num_elements);
	cout << boolalpha << s.isSorted(arr, num_elements) << endl;

	start_time = get_wall_time();
	s.start_time = start_time;
	s.externalMergeSort (arr, 0, num_elements);
	finish_time = get_wall_time();
	cout << "Time needed for external-memory merge sort: "
		<< finish_time - start_time << endl;

	cout << boolalpha << s.isSorted(arr, num_elements) << endl;
	s.makeUnsortedArray (arr, num_elements);
	cout << boolalpha << s.isSorted(arr, num_elements) << endl;
	
	start_time = get_wall_time();
	FunnelSort::sort<TYPE, class Sorter>(arr, arr + num_elements, s);
	finish_time = get_wall_time();
	cout << "Time needed for lazy funnel sort: "
		<< finish_time - start_time << endl;
	cout << boolalpha << s.isSorted(arr, num_elements) << endl;
	
	return 0;
}
