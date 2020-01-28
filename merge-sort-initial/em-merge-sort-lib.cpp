// C++ program to implement external sorting using 
// merge sort 
#include <iostream> 
#include <string>
#include <queue>
using namespace std; 

struct MinHeapNode { 
	// The element to be stored 
	int element; 
	// index of the array from which the element is taken 
	int root; 
}; 

// Comparison object to be used to order the heaps 
struct comp { 
public: 
    bool operator() (const MinHeapNode lhs, const MinHeapNode rhs) const { 
        return lhs.element > rhs.element;
    } 
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
	MinHeapNode heap_arr[k]; 
	priority_queue<MinHeapNode, vector<MinHeapNode>, comp> pq;
	int i; 
	for (i = 0; i < k; i++) { 
		// break if no output file is empty and 
		// index i will be no. of input files 
		if (fscanf(in[i], "%d ", &heap_arr[i].element) != 1) 
			break; 
		heap_arr[i].root = i; // Index of scratch output file 
		pq.push(heap_arr[i]);
	}

	int count = 0; 

	// Now one by one get the minimum element from min 
	// heap and replace it with next element. 
	// run till all filled input files reach EOF 
	while (count != i) { 
		// Get the minimum element and store it in output file 
		MinHeapNode root = pq.top();
		pq.pop();
		fprintf(out, "%d\n", root.element); 

		// Find the next element that will replace current 
		// root of heap. The next element belongs to same 
		// input file as the current min element. 
		if (fscanf(in[root.root], "%d ", &root.element) != 1 ) { 
			root.element = INT_MAX; 
			count++; 
		} 

		// Replace root with next element of input file 
		pq.push(root);
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
	char output_file[] = "output-cpp-lib.txt"; 
	
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
	
	//this is to send the temprary files in a separate folder
	for (int i = 0; i < num_ways; i++) { 
		string cmd = "mv " + to_string(i) + " ./temp-files-cpp";
		const char *command = cmd.c_str();
		system(command);
	}
	return 0; 
} 

