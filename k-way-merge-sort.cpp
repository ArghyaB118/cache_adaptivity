/* C++ program for Merge Sort */
#include<iostream> 
using namespace std;
// Merges two subarrays of arr[]. 
// First subarray is arr[l..m] 
// Second subarray is arr[m+1..r] 
void merge(int arr[], int temp_arr[], int l, int m, int r) { 
	int i, j, k; 
	int n1 = m - l + 1; 
	int n2 = r - m; 

	/* Copy data to temp arrays L[] and R[] */
	for (i = 0; i < n1; i++) 
		temp_arr[i] = arr[l + i]; 
	for (j = 0; j < n2; j++) 
		temp_arr[n1 + j] = arr[m + 1+ j]; 

	/* Merge the temp arrays back into arr[l..r]*/
	i = 0; // Initial index of first subarray 
	j = 0; // Initial index of second subarray 
	k = l; // Initial index of merged subarray 
	while (i < n1 && j < n2) { 
		if (temp_arr[i] <= temp_arr[n1 + j]) { 
			arr[k] = temp_arr[i]; 
			i++; 
		} 
		else { 
			arr[k] = temp_arr[n1 + j]; 
			j++; 
		} 
		k++; 
	} 
	/* Copy the remaining elements of L[], if there are any */
	while (i < n1) { 
		arr[k] = temp_arr[i]; 
		i++; 
		k++; 
	}
	/* Copy the remaining elements of R[], if there are any */
	while (j < n2) { 
		arr[k] = temp_arr[n1 + j]; 
		j++; 
		k++; 
	} 
} 

/* l is for left index and r is right index of the 
sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r, int temp_arr[], int b) { 
	if (l < r && r - l > b) {
		// Same as (l+r)/2, but avoids overflow for large l and h 
		int m = l+(r-l)/2; 
		mergeSort(arr, l, m, temp_arr, b); 
		mergeSort(arr, m+1, r, temp_arr, b);
		merge(arr, temp_arr, l, m, r);
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
		merge(arr, temp_arr, l, m, r); 
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

	mergeSort(arr, 0, num_elements - 1, temp_arr, base_case);
	delete [] temp_arr; temp_arr = NULL; // to deallocate memory for temp array
}

/* Driver program to test above functions */
int main() 
{ 
	const unsigned long long num_elements = 256;
	const unsigned long long base_case = 32;
	int k = 4;
	int arr[num_elements];
	for (unsigned long long i = 0; i < num_elements; i++) {
		arr[i] = rand() % 1000;
  	}
  	cout << "given array is" << endl;  
	printArray(arr, num_elements);
	//rootMergeSort(arr, num_elements, base_case);
	rootMergeSort(arr, &arr[0], &arr[num_elements - 1], base_case, k);
	cout << "sorted array is" << endl;  
	printArray(arr, num_elements);
	return 0; 
} 

