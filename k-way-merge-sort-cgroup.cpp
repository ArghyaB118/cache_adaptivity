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
#include <queue>
#include <algorithm>
using namespace std;


typedef pair<int, pair<int, int> > ppi;

void merge(int arr[], int temp_arr[], int l, int m, int r, int k) {
  int itr = 0;
  priority_queue<ppi, vector<ppi>, greater<ppi> > pq;
  for (int i = 0; i < k; i++) {
    pq.push({arr[l + i*m], {i, 0}});
  }
  while (!pq.empty()) {
    ppi curr = pq.top();
    pq.pop();
    temp_arr[itr] = curr.first; itr++;
    int i = curr.second.first;   
        int j = curr.second.second;
        if (j + 1 < m) 
            pq.push({arr[i*m + j + 1], {i, j + 1}});
  }
  for (int i = 0 ; i < m*k; i++)
    arr[i + l] = temp_arr[i];
}


/* l is for left index and r is right index of the 
sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r, int temp_arr[], int b, int k) { 
  if (l < r && r - l > b) {
    // Same as (l+r)/2, but avoids overflow for large l and h 
    int m = (r - l + 1) / k; 
    for (int i = 0; i < k; ++i) {
      mergeSort(arr, l + i*m, l + i*m + m, temp_arr, b, k); 
    }
    merge(arr, temp_arr, l, m, r, k);
  }
  else if (l < r && r - l <= b) {
    // Same as (l+r)/2, but avoids overflow for large l and h 
    int m = l+(r-l)/2;
    for (int i = 0; i < b; i++) {
      temp_arr[i] = arr[i + l]; 
    }
    sort(temp_arr, temp_arr + b);
    for (int i = 0; i < b; i++) {
      arr[i + l] = temp_arr[i]; 
    }
  }
} 

/* UTILITY FUNCTIONS */
/* Function to print an array */
void printArray(int A[], int size) { 
  for (int i = 0; i < size; i++) 
    cout << A[i] << " ";
  cout << endl; 
} 

//void rootMergeSort(int arr[], int num_elements, int base_case) {
//  int temp_arr[num_elements];
//  mergeSort(arr, 0, num_elements - 1, temp_arr, base_case); 
//}

void rootMergeSort(int arr[], int *arr_first, int *arr_last, int base_case, int k) {
  int num_elements = arr_last - arr_first;
  //int temp_arr[num_elements];

  int* temp_arr = NULL;
  temp_arr = new int[num_elements];

  mergeSort(arr, 0, num_elements, temp_arr, base_case, k);
  delete [] temp_arr; temp_arr = NULL; // to deallocate memory for temp array
}


class Int
{
	public:
		int val;
    Int(int i = 0): val(i) {}
};

class Integer_comparator
{
	public:
		bool operator () (const Int& a, const Int& b) const
		{
			return (a.val < b.val);
		}
};

std::vector<std::string> split(std::string mystring, std::string delimiter)
{
    std::vector<std::string> subStringList;
    std::string token;
    while (true)
    {
        size_t findfirst = mystring.find_first_of(delimiter);
        if (findfirst == std::string::npos) //find_first_of returns npos if it couldn't find the delimiter anymore
        {
            subStringList.push_back(mystring); //push back the final piece of mystring
            return subStringList;
        }
        token = mystring.substr(0, mystring.find_first_of(delimiter));
        mystring = mystring.substr(mystring.find_first_of(delimiter) + 1);
        subStringList.push_back(token);
    }
    return subStringList;
}


std::string exec(std::string cmd){
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        std::cout << "Error. Pipe does not exist!\n";
        exit(1);
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void limit_memory(long memory_in_bytes, const char* string2){
  std::cout << "Entering limit memory function\n";
  std::string string = std::to_string(memory_in_bytes);
  std::string command = std::string("bash -c \"echo ") + string + std::string(" > /var/cgroups/") + string2 + std::string("/memory.limit_in_bytes\"");
  std::cout << "Command: " << command << std::endl;
  int return_code = system(command.c_str());
  //std::cout << "Memory usage: " << exec(std::string("cat /var/cgroups/") + string2 + std::string("/memory.usage_in_bytes")) << std::endl;
  if (return_code != 0){
    std::cout << "Error. Unable to set cgroup memory " << string << " Code: " << return_code << "\n";
    std::cout << "Memory usage: " << exec(std::string("cat /var/cgroups/") + string2 + std::string("/memory.usage_in_bytes")) << std::endl;
  }
  std::cout << "Limiting cgroup memory: " << string << " bytes\n";
}

void print_io_data(std::vector<long>& data, std::string header){
  std::cout << header;
  std::string command = std::string("cat /proc/") + std::to_string(getpid()) + std::string("/io");
  std::string command_output = exec(command);
  std::vector<std::string> splitted_output = split(command_output, " ");
  for (unsigned int i = 0; i < splitted_output.size(); ++i){
    if ( splitted_output[i].find(std::string("read_bytes:")) != std::string::npos) {
      std::cout << "Bytes read: " << (std::stol(splitted_output[i+1]) - data[0]);
      data[0] = std::stol(splitted_output[i+1]);
    }else if ( splitted_output[i].find(std::string("write_bytes:")) != std::string::npos) {
      std::cout << ", Bytes written: " << (std::stol(splitted_output[i+1]) - data[1]) << "\n\n";
      data[1] = std::stol(splitted_output[i+1]);
      break;
    }
  }
}

int main(int argc, char *argv[]){
	srand (time(NULL));

	if (argc < 2){
		std::cout << "Insufficient arguments! Usage: funnel_sort <memory_limit> <cgroup_name>\n";
		exit(1);
	}

//	int fdout;
//
//	if ((fdout = open ("nullbytes", O_RDWR, 0x0777 )) < 0){
//		printf ("can't create nullbytes for writing\n");
//		return 0;
//	}
	const int data_in_megabytes = 1;
  const unsigned long long num_elements = data_in_megabytes*1024*1024/4;

	//Int* array;
	//if (((array = (Int*) mmap(0, sizeof(Int)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (Int*)MAP_FAILED)){
		//	 printf ("mmap error for output with code");
		//	 return 0;
	 //}

 	std::cout << "Running lazy funnel sort on an array of size: " << (int)num_elements << "\n";

	std::vector<long> io_stats = {0,0};
	print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");

	//Integer_comparator comp;
  //for (unsigned long long i = 0; i < num_elements; i++){
    //records[i] = Integer(57-i);
		//array[i] = Int(rand() % 1000);
		//std::cout << array[i].val << "\t";
  //}

  //const unsigned long long num_elements = 512;
  const unsigned long long base_case = 16384;
  int k = 4;
  int arr[num_elements];
  //if (((arr = (int) mmap(0, sizeof(int)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (int)MAP_FAILED)){
    //   printf ("mmap error for output with code");
      // return 0;
   //}

  for (unsigned long long i = 0; i < num_elements; i++) {
    arr[i] = rand() % 10000;
  }

  limit_memory(std::stol(argv[1])*1024*1024,argv[2]);

  cout << "given array is" << endl;  
  printArray(arr, num_elements);

	std::cout << "\n==================================================================\n";

	std::clock_t start;
  double duration;
	start = std::clock();

	print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");

	//FunnelSort::sort<class Int, class Integer_comparator>(&array[0], &array[num_elements], comp);
  /*for (int i = 0; i < num_elements; i++){
    std::cout << array[i].val << "\t";
  }*/

  rootMergeSort(arr, &arr[0], &arr[num_elements - 1], base_case, k);
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	std::cout << std::endl;
	print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");

  cout << "sorted array is" << endl;  
  printArray(arr, num_elements);

	std::cout << "Total sorting time: " << duration << "\n";
//	std::cout << "Size of record: " << sizeof(Int) << std::endl;
  return 0;
}
