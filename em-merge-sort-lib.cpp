// C++ program to implement external sorting using 
// merge sort 
#include <iostream> 
#include <string>
using namespace std; 

struct MinHeapNode { 
	// The element to be stored 
	int element; 
	// index of the array from which the element is taken 
	int root; 
}; 

// Prototype of a utility function to swap two min heap nodes 
// A utility function to swap two elements 
void swap(MinHeapNode* x, MinHeapNode* y) { 
	MinHeapNode temp = *x; 
	*x = *y; 
	*y = temp; 
} 

// A class for Min Heap 
class MinHeap { 
	MinHeapNode* heap_arr; // pointer to array of elements in heap 
	int heap_size;	 // size of min heap 

public: 
	// Constructor: creates a min heap of given size from a given array a[] 
	MinHeap(MinHeapNode arr[], int size) { 
		heap_size = size; 
		heap_arr = arr; // store address of array 
		int i = (size - 1) / 2; 
		while (i >= 0) { 
			MinHeapify(i); 
			i--; 
		} 
	};

	// to get index of left child of node at index i 
	int left(int root) { return (2 * root + 1); } 

	// to get index of right child of node at index i 
	int right(int root) { return (2 * root + 2); } 

	// to get the root 
	MinHeapNode getMin() { return heap_arr[0]; } 

	// to replace root with new node x and heapify() 
	// new root 
	void replaceMin(MinHeapNode x) 
	{ 
		heap_arr[0] = x; 
		MinHeapify(0); 
	} 

	// A recursive method to heapify a subtree with root 
	// at given index. This method assumes that the 
	// subtrees are already heapified 
	void MinHeapify(int i) { 
		int l = left(i); 
		int r = right(i); 
		int smallest = i; 
		if (l < heap_size && heap_arr[l].element < heap_arr[i].element) 
			smallest = l; 
		if (r < heap_size && heap_arr[r].element < heap_arr[smallest].element) 
			smallest = r; 
		if (smallest != i) { 
			swap(&heap_arr[i], &heap_arr[smallest]); 
			MinHeapify(smallest); 
		} 
	};
}; 


// Merges k sorted files. Names of files are assumed 
// to be 1, 2, 3, ... k 
void mergeFiles(char *output_file, int n, int k) { 
	FILE* in[k]; 
	for (int i = 0; i < k; i++) { 
		char fileName[2]; 

		// convert i to string 
		snprintf(fileName, sizeof(fileName), "%d", i); 

		// Open output files in read mode. 
		in[i] = fopen(fileName, "r");
		//in[i] = openFile(fileName, "r"); 
	} 

	// FINAL OUTPUT FILE 
	FILE *out = fopen(output_file, "w");

	// Create a min heap with k heap nodes. Every heap node 
	// has first element of scratch output file 
	MinHeapNode* heap_arr = new MinHeapNode[k]; 
	int i; 
	for (i = 0; i < k; i++) { 
		// break if no output file is empty and 
		// index i will be no. of input files 
		if (fscanf(in[i], "%d ", &heap_arr[i].element) != 1) 
			break; 

		heap_arr[i].root = i; // Index of scratch output file 
	} 
	MinHeap hp(heap_arr, i); // Create the heap 

	int count = 0; 

	// Now one by one get the minimum element from min 
	// heap and replace it with next element. 
	// run till all filled input files reach EOF 
	while (count != i) { 
		// Get the minimum element and store it in output file 
		MinHeapNode root = hp.getMin(); 
		fprintf(out, "%d\n", root.element); 

		// Find the next element that will replace current 
		// root of heap. The next element belongs to same 
		// input file as the current min element. 
		if (fscanf(in[root.root], "%d ", &root.element) != 1 ) { 
			root.element = INT_MAX; 
			count++; 
		} 

		// Replace root with next element of input file 
		hp.replaceMin(root); 
	} 

	// close input and output files 
	for (int i = 0; i < k; i++) 
		fclose(in[i]); 

	fclose(out); 
} 


// Driver program to test above 
int main() { 
	// No. of Partitions of input file. 
	int num_ways = 10; 

	// The size of each partition 
	int run_size = 1000; 

	char input_file[] = "input.txt"; 
	char output_file[] = "output-cpp.txt"; 
	
	// Using a merge-sort algorithm, create the initial runs 
	// and divide them evenly among the output files 
	FILE *in = fopen(input_file, "r"); 

	// output scratch files 
	FILE* out[num_ways]; 
	char fileName[2]; 
	for (int i = 0; i < num_ways; i++) { 
		// convert i to string 
		snprintf(fileName, sizeof(fileName), "%d", i); 

		// Open output files in write mode. 
		out[i] = fopen(fileName, "w");
	} 

	// allocate a dynamic array large enough 
	// to accommodate runs of size run_size 
	int* arr = (int*)malloc(run_size * sizeof(int)); 

	bool more_input = true; 
	int next_output_file = 0; 

	int i; 
	while (more_input) 
	{ 
		// write run_size elements into arr from input file 
		for (i = 0; i < run_size; i++) 
		{ 
			if (fscanf(in, "%d ", &arr[i]) != 1) 
			{ 
				more_input = false; 
				break; 
			} 
		} 

		// sort array using stl sort 
		sort(arr, arr + run_size);

		// write the records to the appropriate scratch output file 
		// can't assume that the loop runs to run_size 
		// since the last run's length may be less than run_size 
		for (int j = 0; j < i; j++) 
			fprintf(out[next_output_file], "%d\n", arr[j]); 

		next_output_file++; 
	} 

	// close input and output files 
	for (int i = 0; i < num_ways; i++) 
		fclose(out[i]); 

	fclose(in);


	mergeFiles(output_file, run_size, num_ways); 
	
	for (int i = 0; i < num_ways; i++) { 
		string cmd = "mv " + to_string(i) + " ./temp-files-cpp";
		const char *command = cmd.c_str();
		system(command);
	}
	return 0; 
} 

