#include "../CacheHelper.h"
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <ctime>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <queue>
#include <algorithm>
#include <fstream>
#include <time.h>
#include <vector>

using namespace std;
#define TYPE int

typedef pair<int, pair<int, int> > ppi;
//std::clock_t start;
time_t start, finish; // introduced to measure wall clock time
double duration;
std::ofstream out_sorting, logfile;
char* datafile;
std::vector<long> io_stats = {0,0};
int type_of_run, data_in_megabytes, memory_given_MB, k_logical, logical_block_size_KB, actual_block_size_KB;
vector<int> random_memory;
unsigned long long num_elements, base_case, memory;

/* M/B-way merge, to be precise M/4B */
void merge(int arr[], int temp_arr[], unsigned long long l, unsigned long long m, unsigned long long r) {
  unsigned long long itr = 0ULL;
  priority_queue<ppi, vector<ppi>, greater<ppi> > pq;
  for (int i = 0; i < k_logical; i++) {
    for (unsigned long long j = 0; j < 1024ULL * actual_block_size_KB / sizeof(TYPE); j++) {
        pq.push({arr[l + i*m + j], {i, j}});
    }
  }
  //cout << "done now" << endl;
  while (!pq.empty()) {
    ppi curr = pq.top();// cout << sizeof(curr) << endl;
    pq.pop();
    temp_arr[itr] = curr.first; itr++;
    int i = curr.second.first;   
    unsigned long long j = curr.second.second;
    if (j + 1 < m && (j + 1) % (1024ULL * actual_block_size_KB / sizeof(TYPE)) == 0) {
      //cout << "here " << j + 1 << " " << 1048576ULL * actual_block_size_MB / sizeof(TYPE) << endl;
      for (unsigned long long p = 0ULL; p < 1024ULL * actual_block_size_KB / sizeof(TYPE); ++p) {
        pq.push({arr[m*i + j + p], {i, j + p + 1}});
      }
    } 
  }
  for (unsigned long long i = 0 ; i < m * k_logical; i++)
    arr[i + l] = temp_arr[i];
}

/* l is for left index and r is right index of the 
sub-array of arr to be sorted */
void mergeSort(int arr[], unsigned long long l, unsigned long long r, int temp_arr[]) { 
  if (l < r && r - l + 1 > base_case) {
    logfile << "recursive call" << endl;
    unsigned long long m = (r - l + 1) / k_logical; 
    for (int i = 0; i < k_logical; ++i) {
      mergeSort(arr, l + i*m, l + i*m + m - 1, temp_arr); 
    }
    // constant memory profile
    if (type_of_run == 1) {
      logfile << "merging elements from " << l << " to " << r << endl;
      merge(arr, temp_arr, l, m, r); 
    }
  }
  else if (l < r && r - l + 1 <= base_case) {
    logfile << "in-memory sort" << endl;
    actual_block_size_KB = (r - l + 1) * sizeof(TYPE) / 1024;
    sort(arr + l, arr + (r + 1));
  }
} 

/* root function to call merge sort */
void rootMergeSort(int arr[], int *arr_first, int *arr_last) {
  int* temp_arr = NULL;
  temp_arr = new int[num_elements];

  mergeSort(arr, 0, num_elements - 1, temp_arr);
  delete [] temp_arr; temp_arr = NULL; // to deallocate memory for temp array
}


int main(int argc, char *argv[]){
  srand (time(NULL));
  logfile = std::ofstream("log.txt",std::ofstream::out | std::ofstream::app);
  if (argc < 7){
    std::cout << "Insufficient arguments! Usage: ./a.out <memory_limit> <data_size> <cgroup_name> <type> <k_logical> <block_size>";
    //type = 1 for constant memory and 2 for worse case memory
    //k_logical = 0 for k = m / 4B where B = 4k, 1 for (M / 4B) where B is logical block, otherwise it's directly given as integer
    exit(1);
  }
  memory_given_MB = atoi(argv[1]); data_in_megabytes = atoi(argv[2]);
  datafile = new char[strlen(argv[4]) + 1](); strncpy(datafile,argv[4],strlen(argv[4]));
  k_logical = atoi(argv[5]); logical_block_size_KB = atoi(argv[6]);
  num_elements = 1048576ULL * data_in_megabytes / sizeof(TYPE);
  base_case = 1024ULL * logical_block_size_KB / sizeof(TYPE);
  int fdout, ipcfd;
  if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
    printf ("can't create nullbytes for writing\n");
    return 0;
  }
  // string manipulation for cgroup name
  //cgroup_name = new char[strlen(argv[3]) + 1](); strncpy(cgroup_name,argv[3],strlen(argv[3]));
  //some checks are required here
  if ((k_logical + 1) * logical_block_size_KB / 1024 > memory_given_MB) {
    cout << "constraint not met" << endl;
    return 0;
  }
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
  logfile << "Running " << k_logical <<"-way merge sort on an array of size: " << num_elements << " with base case " << base_case << std::endl;
  TYPE* arr;
  if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
    printf ("mmap error for output with code");
    return 0;
  }
  memory = 1048576ULL * memory_given_MB;
  //CacheHelper::limit_memory(memory, cgroup_name);
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");
  start = time(NULL); 
  rootMergeSort(arr, &arr[0], &arr[num_elements - 1]);
  finish = time(NULL);
  //duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  duration = (finish - start);
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");
  logfile << "Total sorting time: " << duration << "\n";
  out_sorting = std::ofstream("out-sorting.txt",std::ofstream::out | std::ofstream::app);
  out_sorting << "Merge sort, logical block size in KB is " << logical_block_size_KB << ", actual block size in KB is " << actual_block_size_KB << "," << duration << "," << io_stats[0] << "," << io_stats[1] << std::endl;
  //introduced code for checking the accuracy of sorting result
  std::ofstream test_out = std::ofstream("test_out.txt", std::ofstream::out);
  for (unsigned long long i = 1 ; i < num_elements; i++) {
   if ((int)arr[i - 1] > (int)arr[i]) {
      logfile << "bad result " << (unsigned long long)i << " " << (int)arr[i - 2] << " " << (int)arr[i - 1] << " " << (int)arr[i] << " " << (int)arr[i + 1] << endl;
   }
  }
  out_sorting.close(); logfile.close();
  return 0;
}
