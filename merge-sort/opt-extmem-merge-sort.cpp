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
using namespace std;
#define TYPE int

typedef pair<int, pair<int, int> > ppi;
std::clock_t start;
double duration;
std::ofstream out;
std::ofstream out_sorting;
char* cgroup_name;
std::vector<long> io_stats = {0,0};
int type_of_run, data_in_megabytes, memory_given_MB;
unsigned long long num_elements, base_case;

/* UTILITY FUNCTIONS */
/* Function to print an array */
void printArray(int A[], int size) { 
  for (int i = 0; i < size; i++) 
    cout << A[i] << " ";
  cout << endl; 
} 
/* M/B-way merge, to be precise M/4B */
void merge(int arr[], int temp_arr[], int l, int m, int r, int k) {
  int itr = 0;
  priority_queue<ppi, vector<ppi>, greater<ppi> > pq;
  for (int i = 0; i < k; i++) {
	for (int j = 0; j < 1024; j++){
   		 pq.push({arr[l + i*m + j], {i, j}});
	}
  }
  while (!pq.empty()) {
    ppi curr = pq.top();// cout << sizeof(curr) << endl;
    pq.pop();
    temp_arr[itr] = curr.first; itr++;
    int i = curr.second.first;   
        int j = curr.second.second;
        if (j + 1 < m && (j + 1) % 1024 == 0) {
		for (int p = 0; p < 1024; p++) {
			pq.push({arr[i*m + j + p], {i, j + p + 1}});
		}
	} 
  }
  for (int i = 0 ; i < m * k; i++)
    arr[i + l] = temp_arr[i];
}

/* l is for left index and r is right index of the 
sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r, int temp_arr[], int b, int k) { 
  if (l < r && r - l + 1 > b) {
    int m = (r - l + 1) / k; 
    for (int i = 0; i < k; ++i) {
      mergeSort(arr, l + i*m, l + i*m + m - 1, temp_arr, b, k); 
    }
    if (type_of_run == 1) {
      cout << "merging elements from " << l << " to " << r << endl;
      merge(arr, temp_arr, l, m, r, k); 
    }
    else if (type_of_run == 2) {
      unsigned long long memory = (r - l + 1) * sizeof(TYPE) + memory_given_MB * 1024 * 1024;
  	  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  	  out << duration << " " << memory << std::endl;
  	  CacheHelper::limit_memory(memory, cgroup_name);
  	  merge(arr, temp_arr, l, m, r, k);
  	  memory = memory_given_MB * 1024 * 1024;
  	  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  	  out << duration << " " << memory << std::endl;
  	  CacheHelper::limit_memory(memory, cgroup_name);
    }
  }
  else if (l < r && r - l + 1 <= b) {
  	sort(arr + l, arr + (r + 1));
  }
} 

/* root function to call merge sort */
void rootMergeSort(int arr[], int *arr_first, int *arr_last, int base_case, int k) {
  int num_elements = arr_last - arr_first;

  int* temp_arr = NULL;
  temp_arr = new int[num_elements];

  mergeSort(arr, 0, num_elements, temp_arr, base_case, k);
  delete [] temp_arr; temp_arr = NULL; // to deallocate memory for temp array
}


int main(int argc, char *argv[]){
	srand (time(NULL));

	if (argc < 5){
		std::cout << "Insufficient arguments! Usage: ./opt-extmem-merge-sort <memory_limit> <data_size> <cgroup_name> <type> <k_logical>\n";
		//type = 1 for constant memory and 2 for worse case memory
		//k_logical = 0 for optimized choice of k, otherwise it's directly given as integer
		exit(1);
	}

	int fdout;

	if ((fdout = open ("/home/arghya/EM-MergeSort/merge-sort/nullbytes", O_RDWR, 0x0777 )) < 0){
		printf ("can't create nullbytes for writing\n");
		return 0;
	}
  int k, k_logical = atoi(argv[5]);
  type_of_run = atoi(argv[4]);
  data_in_megabytes = atoi(argv[2]); memory_given_MB = atoi(argv[1]);	
  num_elements = (data_in_megabytes / sizeof(TYPE)) * 1024 * 1024; //num_elements = data_in_megabytes; cout << num_elements << endl;
  base_case = memory_given_MB * 1024 * 1024 / sizeof(TYPE);
  cgroup_name = new char[strlen(argv[3]) + 1](); strncpy(cgroup_name,argv[3],strlen(argv[3]));
  if (k_logical == 0) 
     k = (memory_given_MB / 4) * 256; // memory_given_MB * 1024 * 1024 / (4 * 1024);
  else
     k = k_logical;
 	std::cout << "\n==================================================================\n";
  	CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
  std::cout << "Running " << k <<"-way merge sort on an array of size: " << num_elements << " with base case " << base_case << std::endl;
  TYPE* arr;
  if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
      printf ("mmap error for output with code");
      return 0;
  }
	start = std::clock();
  out = std::ofstream("mem_profile.txt", std::ofstream::out); 
  out << duration << " " << memory_given_MB * 1024 * 1024 << std::endl;
  CacheHelper::limit_memory(memory_given_MB * 1024 * 1024, cgroup_name);
	std::cout << "\n==================================================================\n";
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");

  rootMergeSort(arr, &arr[0], &arr[num_elements - 1], base_case, k);
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	std::cout << "\n==================================================================\n";
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");

	std::cout << "Total sorting time: " << duration << "\n";
  out_sorting = std::ofstream("/home/arghya/EM-MergeSort/out-sorting.txt",std::ofstream::out | std::ofstream::app);
  out_sorting << "Merge sort" << "," << duration << "," << io_stats[0] << "," << io_stats[1] << std::endl;
  out.close(); out_sorting.close();
  //introduced code for checking the accuracy of sorting result
  CacheHelper::limit_memory(1024 * 1024 * 1024, cgroup_name);
  for (int i = 1 ; i < num_elements; i++) {
	 if ((int)arr[i - 1] > (int)arr[i]) {
		  cout << "bad result " << (int)i << " " << (int)arr[i - 2] << " " << (int)arr[i - 1] << " " << (int)arr[i] << " " << (int)arr[i + 1] << endl;
//		  break; 
	 }
  }
 return 0;
}
