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
char* cgroup_name;
std::vector<long> io_stats = {0,0};

/* UTILITY FUNCTIONS */
/* Function to print an array */
void printArray(int A[], int size) { 
  for (int i = 0; i < size; i++) 
    cout << A[i] << " ";
  cout << endl; 
} 

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
void mergeSort(int arr[], int l, int r, int temp_arr[], int b, int k, int data_in_megabytes, int memory_given_MB) { 
  //cout << l << " " << r << endl;
  if (l < r && r - l + 1 > b) {
    // Same as (l+r)/2, but avoids overflow for large l and h 
    int m = (r - l + 1) / k; 
    for (int i = 0; i < k; ++i) {
      mergeSort(arr, l + i*m, l + i*m + m - 1, temp_arr, b, k, data_in_megabytes, memory_given_MB); 
    }
//	cout << "here" << endl;
    //CacheHelper::limit_memory(4 * memory_given_MB*1024*1024 + 8192, cgroup_name);  
    merge(arr, temp_arr, l, m, r, k);
  }
  else if (l < r && r - l + 1 <= b) {
//	cout << "i'm here" << endl;
    for (int i = 0; i < r - l + 1; i++) {
      temp_arr[i] = arr[i + l]; 
    }
    sort(temp_arr, temp_arr + r - l + 1);
    for (int i = 0; i < r - l + 1; i++) {
      arr[i + l] = temp_arr[i]; 
    }
//      cout << "leaving here" << endl; 
  }
} 



void rootMergeSort(int arr[], int *arr_first, int *arr_last, int base_case, int k, int data_in_megabytes, int memory_given_MB) {
  int num_elements = arr_last - arr_first;
  //int temp_arr[num_elements];

  int* temp_arr = NULL;
  temp_arr = new int[num_elements];

  mergeSort(arr, 0, num_elements, temp_arr, base_case, k, data_in_megabytes, memory_given_MB);
  delete [] temp_arr; temp_arr = NULL; // to deallocate memory for temp array
}


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
  const unsigned long long num_elements = data_in_megabytes * 1024 * 1024 / 4;
  const unsigned long long base_case = memory_given_MB * 1024 * 1024 / 4;
  cgroup_name = new char[strlen(argv[3]) + 1](); strncpy(cgroup_name,argv[3],strlen(argv[3]));
  //int k  = (int)num_elements / (int)base_case;
  int k = memory_given_MB * 256 / 8; // memory_given_MB * 1024 * 1024 / (4 * 1024);
	std::cout << "\n==================================================================\n";
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
  std::cout << "Running " << k <<"-way merge sort on an array of size: " << (int)num_elements << " with base case " << (int)base_case << std::endl;
  TYPE* arr;
  if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
      printf ("mmap error for output with code");
      return 0;
  }
	start = std::clock();
  out = std::ofstream("mem_profile.txt", std::ofstream::out); 
  out << duration << " " << atoi(argv[1])*1024*1024 << std::endl;
  CacheHelper::limit_memory(std::stol(argv[1])*1024*1024, argv[3]);
	std::cout << "\n==================================================================\n";
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");

  rootMergeSort(arr, &arr[0], &arr[num_elements - 1], base_case, k, data_in_megabytes, memory_given_MB);
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	std::cout << "\n==================================================================\n";
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");

//  cout << "sorted array is" << endl;  
//  printArray(arr, num_elements);
	std::cout << "Total sorting time: " << duration << "\n";
 out.close();
//introduced code for checking the accuracy of sorting result
  for (int i = 0 ; i < num_elements; i++) {
	 if (arr[i] > arr[i + 1]) {
		  cout << "bad result" << endl;
		  break; 
	 }
  } 
 return 0;
}
