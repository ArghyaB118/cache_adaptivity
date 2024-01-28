#include "../CacheHelper.h"
#include "funnelSort.h"
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <ctime>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <time.h>
#include <stdio.h>
using namespace std;
#define TYPE int
time_t start, finish; // introduced to measure wall clock time
double duration;
std::ofstream out_sorting;
std::vector<long> io_stats = {0,0};
int data_MiB;
unsigned long long num_elements;

class Integer_comparator
{
	public:
		bool operator () (const int& a, const int& b) const
		{
			return (a < b);
		}
};


int main(int argc, char *argv[]){
	srand (time(NULL));
	if (argc < 1){
		std::cout << "Insufficient arguments! Usage: ./funnel_sort <data_size>\n";
		exit(1);
	}
	data_MiB = atoi(argv[1]);
  	num_elements = 1048576ULL * data_MiB / sizeof(TYPE);
 	
 	char* datafile = new char[strlen(argv[2]) + 1](); strncpy(datafile,argv[2],strlen(argv[2]));
	int fdout;
	if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0) {
		printf ("can't create nullbytes for writing\n");
		return 0;
	}

  	CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
	std::cout << "Running lazy funnel sort on an array of size: " << num_elements << "\n";
	TYPE* arr;
	if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
	printf ("mmap error for output with code\n");
	return 0;
	}
	Integer_comparator comp;
	start = time(NULL); 
	// FunnelSort::sort<int, class Integer_comparator>(&arr[0], &arr[num_elements-1], comp);
	FunnelSort::sort<int, class Integer_comparator>(arr, arr + num_elements, comp);
	finish = time(NULL);
	duration = (finish - start);
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting @@@@@ \n");
	std::cout << "Total sorting time: " << duration << "\n";
  	out_sorting = std::ofstream("out-sorting.txt",std::ofstream::out | std::ofstream::app);
  	out_sorting << "FunnelSort," << duration << "," << (float)io_stats[0] / 1000000.0 << "," << io_stats[1] / 1000000.0 <<  "," << (float)(io_stats[0] + io_stats[1]) / 1000000.0 << std::endl;
 	out_sorting.close();
 	return 0;
}
