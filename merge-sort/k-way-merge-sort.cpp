/* C++ program for Merge Sort */
#include <iostream> 
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
//	int temp_arr[num_elements];
//	mergeSort(arr, 0, num_elements - 1, temp_arr, base_case); 
//}

void rootMergeSort(int arr[], int *arr_first, int *arr_last, int base_case, int k) {
	int num_elements = arr_last - arr_first;
	//int temp_arr[num_elements];

	int* temp_arr = NULL;
	temp_arr = new int[num_elements];

	mergeSort(arr, 0, num_elements, temp_arr, base_case, k);
	delete [] temp_arr; temp_arr = NULL; // to deallocate memory for temp array
}

/* Driver program to test above functions */
int main() 
{ 
	const unsigned long long num_elements = 512;
	const unsigned long long base_case = 32;
	int k = 4;
	int arr[num_elements];
	for (unsigned long long i = 0; i < num_elements; i++) {
		arr[i] = rand() % 10000;
  	}
  	cout << "given array is" << endl;  
	printArray(arr, num_elements);
	//rootMergeSort(arr, num_elements, base_case);
	rootMergeSort(arr, &arr[0], &arr[num_elements - 1], base_case, k);
	cout << "sorted array is" << endl;  
	printArray(arr, num_elements);
	return 0; 
} 

