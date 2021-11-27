#include "../CacheHelper.h"
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
#include <cstring>
#include <fcntl.h>
#include <queue>
#include <algorithm>
#include <fstream>
#include <time.h>
#include <stdio.h>
using namespace std;
#define TYPE int

typedef pair<int, pair<int, int> > ppi;
// std::clock_t start;
time_t start, finish; // introduced to measure wall clock time
double duration;
std::ofstream out_sorting, out;
std::string type_of_run;
std::vector<long> io_stats = {0,0};
int data_MiB, memory_MiB, fanout, logical_block_KiB, actual_block_KiB;
unsigned long long num_elements, base_case, memory;
unsigned long long* dst;
char* cgroup;


/* UTILITY FUNCTIONS */
/* Function to print an array */
void printArray(int A[], int size) { 
  for (int i = 0; i < size; i++) 
    cout << A[i] << " ";
  cout << endl; 
} 
bool isSorted(int arr[], int num_elements) {
  for (unsigned long long i = 1 ; i < num_elements; i++) {
    if ((int)arr[i - 1] > (int)arr[i]) {
      return false;
    }
  }
  return true;
}

/* M/B-way merge, to be precise M/4B */
void merge(int arr[], int temp_arr[], unsigned long long l, unsigned long long m, unsigned long long r) {
  if (verbose) { std::cout << "Merging elements from: " << l << " to: " << r << std::endl; }
  unsigned long long itr = 0ULL;
  priority_queue<ppi, vector<ppi>, greater<ppi> > pq;
  for (int i = 0; i < fanout; i++) {
    for (unsigned long long j = 0; j < 1024ULL * actual_block_KiB / sizeof(TYPE); j++) {
        pq.push({arr[l + i*m + j], {i, j}});
    }
  }
  while (!pq.empty()) {
    ppi curr = pq.top();
    pq.pop();
    temp_arr[itr] = curr.first; itr++;
    int i = curr.second.first;   
    unsigned long long j = curr.second.second;
    if (j + 1 < m && (j + 1) % (1024ULL * actual_block_KiB / sizeof(TYPE)) == 0) {
      for (unsigned long long p = 0ULL; p < 1024ULL * actual_block_KiB / sizeof(TYPE); ++p) {
        pq.push({arr[m*i + j + p], {i, j + p + 1}});
      }
    } 
  }
  for (unsigned long long i = 0 ; i < m * fanout; i++)
    arr[i + l] = temp_arr[i];
}

/* l is for left index and r is right index of the 
sub-array of arr to be sorted */
void mergeSort(int arr[], unsigned long long l, unsigned long long r, int temp_arr[]) { 
  if (l < r && r - l + 1 > base_case) {
    unsigned long long m = (r - l + 1) / fanout; 
    for (int i = 0; i < fanout; ++i) {
      mergeSort(arr, l + i*m, l + i*m + m - 1, temp_arr); 
    }
    if (type_of_run == "constant") {
      merge(arr, temp_arr, l, m, r); 
    }
    // adaptive adversarial memory profile
    else if (type_of_run == "adversarial") {
      memory = 1048576ULL * memory_MiB + (r - l + 1) * sizeof(TYPE);
      duration = time(NULL) - start;
      out << duration << " " << memory << std::endl;
      CacheHelper::limit_memory(memory, cgroup);
      merge(arr, temp_arr, l, m, r);
      memory = 1048576ULL * memory_MiB;
      duration = time(NULL) - start;
      out << duration << " " << memory << std::endl;
      CacheHelper::limit_memory(memory, cgroup);
    }
    // adaptive benevolent memory profile
    else if (type_of_run == "benevolent") {
      memory = 1048576ULL * memory_MiB / 2;
      duration = time(NULL) - start;
      out << duration << " " << memory << std::endl;
      CacheHelper::limit_memory(memory, cgroup);
      merge(arr, temp_arr, l, m, r);
      memory = 1048576ULL * memory_MiB;
      duration = time(NULL) - start;
      out << duration << " " << memory << std::endl;  
      CacheHelper::limit_memory(memory, cgroup);
    }
  }
  else if (l < r && r - l + 1 <= base_case) {
    actual_block_KiB = (r - l + 1) * sizeof(TYPE) / 1024ULL;
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


void memProfSetup() {
  if (type_of_run == "adversarial") {
    std::string temp = "mem_profiles/sorting_profile_adversarial_" + std::to_string(data_MiB) + ".txt";
    out = std::ofstream(temp, std::ofstream::out);
  }
  else if (type_of_run == "benevolent") {
    std::string temp = "mem_profiles/sorting_profile_benevolent_" + std::to_string(data_MiB) + ".txt";
    out = std::ofstream(temp, std::ofstream::out);
  }
}

int main(int argc, char *argv[]){
	srand (time(NULL));
	if (argc < 5){
		std::cout << "Insufficient arguments! Usage: ./a.out <data_size> <fanout> <block_size>";
		exit(1);
	}

	int fdout;
	if ((fdout = open ("data_files/nullbytes", O_RDWR, 0x0777 )) < 0){
		printf ("can't create nullbytes for writing\n");
		return 0;
	}
  
  // read the input variables
  type_of_run = argv[1];
  data_MiB = atoi(argv[2]); memory_MiB = atoi(argv[3]); fanout = atoi(argv[4]); logical_block_KiB = atoi(argv[5]);
  cgroup = new char[strlen(argv[6]) + 1](); strncpy(cgroup,argv[6],strlen(argv[6]));
  
  num_elements = 1048576ULL * data_MiB / sizeof(TYPE); cout << num_elements << endl;
  memory = 1048576ULL * memory_MiB; 
  base_case = 1024ULL * logical_block_KiB / sizeof(TYPE);
  if (type_of_run == "adversarial" || type_of_run == "benevolent") {
    memProfSetup();
  }
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
  std::cout << "Running " << fanout <<"-way merge sort on an array of size: " << num_elements << " with base case " << base_case << std::endl;
  // mmap the unsorted array in the nullbytes
  TYPE* arr;
  if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)) {
    printf ("mmap error for output with code");
    return 0;
  }
  std::cout << std::boolalpha << isSorted(arr, num_elements) << std::endl;
  start = time(NULL); 
  rootMergeSort(arr, &arr[0], &arr[num_elements - 1]);
  finish = time(NULL);
  duration = (finish - start);
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");

	std::cout << "Total sorting time: " << duration << "\n";
  out_sorting = std::ofstream("out-sorting.txt",std::ofstream::out | std::ofstream::app);
  out_sorting << "MergeSort,logical_block_KiB=" << logical_block_KiB << ",actual_block_KiB=" << actual_block_KiB << ",fanout=" << fanout 
  << "," << duration << "," << (float)io_stats[0] / 1000000.0 << "," << (float)io_stats[1] / 1000000.0 << "," << (float)(io_stats[0] + io_stats[1]) / 1000000.0 << std::endl;
  out_sorting.close(); out.close();
  //introduced code for checking the accuracy of sorting result
  std::cout << std::boolalpha << isSorted(arr, num_elements) << std::endl;
  return 0;
}
