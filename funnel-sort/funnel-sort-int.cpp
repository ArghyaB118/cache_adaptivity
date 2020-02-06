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
using namespace std;
#define TYPE int
std::clock_t start;
double duration;
std::ofstream out;
char* cgroup_name;
std::vector<long> io_stats = {0,0};

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

	int fdout;

	if ((fdout = open ("/home/arghya/EM-MergeSort/merge-sort/nullbytes", O_RDWR, 0x0777 )) < 0){
		printf ("can't create nullbytes for writing\n");
		return 0;
	}
	const int data_in_megabytes = atoi(argv[2]);
  const int memory_given_MB = atoi(argv[1]);
  cgroup_name = new char[strlen(argv[3]) + 1](); strncpy(cgroup_name,argv[3],strlen(argv[3]));
  const unsigned long long num_elements = data_in_megabytes*1024*1024/4;
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");

  std::cout << "Running lazy funnel sort on an array of size: " << (int)num_elements << "\n";
	TYPE* arr;
    if (((arr = (TYPE*) mmap(0, sizeof(int)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
        printf ("mmap error for output with code");
        return 0;
    }

	Integer_comparator comp;
  start = std::clock();
//  out = std::ofstream("mem_profile.txt", std::ofstream::out); 
//  out << duration << " " << atoi(argv[1])*1024*1024 << std::endl;
  CacheHelper::limit_memory(memory_given_MB * 1024 * 1024, cgroup_name);
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");

  FunnelSort::sort<int, class Integer_comparator>(&arr[0], &arr[num_elements], comp);
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");
	std::cout << "Total sorting time: " << duration << "\n";
 out.close(); 
 return 0;
}
