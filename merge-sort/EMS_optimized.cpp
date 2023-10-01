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

//typedef pair<int, unsigned long long> ppi;
typedef pair<int, int> ppi;
time_t start, finish; // introduced to measure wall clock time
double duration;
std::ofstream out_sorting;
std::vector<long> io_stats = {0,0};
int data_MiB, fanout, logical_block_KiB, actual_block_KiB;
unsigned long long num_elements, base_case, actual_base_case;
bool comparator(int i, int j) { return (i < j); }

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

void Merge(int *arr_first, int *arr_last, unsigned long long size, int temp_arr[]) {
// cout << "Merging with size: " << size << endl;
  unsigned long long itr = 0ULL;
  priority_queue<ppi, vector<ppi>, greater<ppi> > pq;
  for (int i = 0; i < fanout; i++)
    for (unsigned long long j = 0ULL; j <= actual_base_case; j++)
      pq.push({*(arr_first + i * size / fanout + j), (i * size / fanout + j)});
//  cout << " initial runs read" << endl;
  while (!pq.empty()) {
    ppi curr = pq.top(); pq.pop();
    temp_arr[itr] = curr.first; itr++;
    unsigned long long i = curr.second;  
    if (i + 1 < size && (i + 1) % actual_base_case == 0 && (i + 1) % (size / fanout) != 0) {
      int run_number = (i + 1) % (size / fanout);
      for (unsigned long long p = 0ULL; p < actual_base_case; ++p)
        pq.push({*(arr_first + i + p + 1), (i + p + 1)});
    } 
  }
//  cout << "Merge done" << endl;
  for (unsigned long long i = 0; i < size; i++)
    *(arr_first + i) = temp_arr[i];
}


/* root function to call merge sort */
void rootMergeSort(int *arr_first, int *arr_last, int temp_arr[]) {
  unsigned long long size = arr_last - arr_first + 1;
//  cout << "Merge sort with size: " << size << endl;
  //cout << arr_first << " " << *arr_first << " " << *(arr_first + 1) << " " << &arr_first << " " << &arr_first + 1 << endl;
  if (size > base_case) {
    for (int i = 0; i < fanout; ++i)
      rootMergeSort(arr_first + i*size/fanout, arr_first + i*size/fanout + size/fanout - 1, temp_arr);
    Merge(arr_first, arr_last, size, temp_arr);
  }
  else {
    actual_base_case = size; actual_block_KiB = size * sizeof(TYPE) / 1024ULL;
    sort(arr_first, arr_last, comparator);
  }
}


int main(int argc, char *argv[]){
	srand (time(NULL));
	if (argc < 3){
		std::cout << "Insufficient arguments! Usage: ./a.out <data_size> <fanout> <block_size>";
		exit(1);
	}

  // read the input variables
  data_MiB = atoi(argv[1]); fanout = atoi(argv[2]); logical_block_KiB = atoi(argv[3]);
  num_elements = 1048576ULL * data_MiB / sizeof(TYPE); // cout << num_elements << endl;
  // base case selection
  base_case = 1024ULL * logical_block_KiB / sizeof(TYPE);

  char* datafile = new char[strlen(argv[4]) + 1](); strncpy(datafile,argv[4],strlen(argv[4]));
  int fdout;
  if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
    printf ("can't create nullbytes for writing\n");
    return 0;
  }

  CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
  std::cout << "Running " << fanout <<"-way merge sort on an array of size: " << num_elements << " with base case " << base_case << std::endl;
  // mmap the unsorted array in the nullbytes
  TYPE* arr;
  if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
    printf ("mmap error for output with code");
    return 0;
  }
  // Check if the initial array is unsorted
  std::cout << std::boolalpha << isSorted(arr, num_elements) << std::endl;
  
  int* temp_arr = NULL; temp_arr = new int[num_elements];

  // Execute the program
  start = time(NULL); 
  rootMergeSort(&arr[0], &arr[num_elements - 1], temp_arr);
  finish = time(NULL);
  duration = (finish - start);
  // print the results
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting @@@@@ \n");
  
    delete [] temp_arr; temp_arr = NULL;

	std::cout << "Total sorting time: " << duration << "\n";
  out_sorting = std::ofstream("out-sorting.txt",std::ofstream::out | std::ofstream::app);
  out_sorting << "MergeSort,logical_block_KiB=" << logical_block_KiB << ",actual_block_KiB=" << actual_block_KiB << ",fanout=" << fanout 
  << "," << duration << "," << (float)io_stats[0] / 1000000.0 << "," << (float)io_stats[1] / 1000000.0 <<  "," << (float)(io_stats[0] + io_stats[1]) / 1000000.0 << std::endl;
  out_sorting.close();
  // Check the accuracy of sorting result
  std::cout << std::boolalpha << isSorted(arr, num_elements) << std::endl;
  return 0;
}
