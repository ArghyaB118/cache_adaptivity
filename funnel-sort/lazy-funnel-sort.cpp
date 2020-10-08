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
//std::clock_t start;
time_t start, finish; // introduced to measure wall clock time
double duration;
std::ofstream out_sorting;
char* datafile;
std::vector<long> io_stats = {0,0};
int data_in_megabytes, memory_given_MB;
unsigned long long num_elements, memory;

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

	if (argc < 3){
		std::cout << "Insufficient arguments! Usage: funnel_sort <memory_limit> <cgroup_name>\n";
		exit(1);
	}
  data_in_megabytes = atoi(argv[2]); memory_given_MB = atoi(argv[1]);
  datafile = new char[strlen(argv[3]) + 1](); strncpy(datafile,argv[3],strlen(argv[3]));
	int fdout;
	if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
		printf ("can't create nullbytes for writing\n");
		return 0;
	}
  num_elements = 1048576ULL * data_in_megabytes / sizeof(TYPE);
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
  std::cout << "Running lazy funnel sort on an array of size: " << num_elements << "\n";
	TYPE* arr;
  if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
    printf ("mmap error for output with code\n");
    return 0;
  }
	Integer_comparator comp;
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");
  start = time(NULL); 
  FunnelSort::sort<int, class Integer_comparator>(&arr[0], &arr[num_elements-1], comp);
  finish = time(NULL);
  //duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  duration = (finish - start);
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");
	std::cout << "Total sorting time: " << duration << "\n";
  out_sorting = std::ofstream("out-sorting.txt",std::ofstream::out | std::ofstream::app);
  out_sorting << "Funnel sort" << "," << duration << "," << io_stats[0] << "," << io_stats[1] << std::endl;
  out_sorting.close();
  return 0;
}
