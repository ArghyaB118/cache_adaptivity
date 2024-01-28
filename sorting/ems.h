#include <iostream>
#include <algorithm>
#include <queue>

#include "../tools/config.h"
#include "../tools/util.h"
#include "../CacheHelper.h"

using namespace std;

typedef pair<TYPE, TYPE> indexValue;

class Sorter {
private:
	TYPE num_elements;
	int fanout;
	int base_case;
	int actual_base_case;
	TYPE memory_bytes;
public:
	double start_time;
	Sorter (TYPE num_elements, int fanout, int base_case, TYPE memory_bytes) {
		this->num_elements = num_elements;
		this->fanout = fanout;
		this->base_case = base_case;
		this->memory_bytes = memory_bytes;
	}
	bool operator () (const TYPE& a, const TYPE& b) const { return (a < b); }
	void makeUnsortedArray (TYPE arr[], TYPE num_elements);
	void binaryMergeSort (TYPE arr[], TYPE l, TYPE r);
	bool isSorted (TYPE arr[], TYPE num_elements);
	void printArray (TYPE arr[], TYPE arr_size);

	void kWayMerge (TYPE arr[], TYPE l, TYPE r);
	void externalMergeSort (TYPE arr[], TYPE l, TYPE r);
};

void Sorter::printArray (TYPE arr[], TYPE arr_size) { 
	for (TYPE i = 0; i < arr_size; i++) 
    	cout << arr[i] << " ";
	cout << endl; 
}

bool Sorter::isSorted (TYPE arr[], TYPE num_elements) {
  for (TYPE i = 1 ; i < num_elements; i++) {
    if (arr[i - 1] > arr[i]) {
      return false;
    }
  }
  return true;
}

void Sorter::makeUnsortedArray (TYPE arr[], TYPE num_elements) {
	srand((unsigned) time(NULL));
	for (TYPE i = 0; i < num_elements; i++)
	    arr[i] = 1 + rand() % range;
}

void Sorter::binaryMergeSort (TYPE arr[], TYPE l, TYPE r) {
	if (r < l) {
		cout << "error: r < l" << endl;
		return;
	}
	if (r - l <= this->base_case)
		sort(arr + l, arr + r);
	else {
		TYPE mid = l + (r - l) / 2;
		binaryMergeSort(arr, l, mid);
    	binaryMergeSort(arr, mid, r);
		std::inplace_merge (arr + l, arr + mid, arr + r);
	}
}

/* M/B-way merge, to be precise M/2B */
void Sorter::kWayMerge (TYPE arr[], TYPE l, TYPE r) {
	// cout << l << "here" << r << endl;
	TYPE run_size = (r - l) / this->fanout;
	TYPE itr = 0;
	priority_queue<indexValue, vector<indexValue>, greater<indexValue> > pq;
	TYPE* tmp_arr = NULL;
	tmp_arr = new TYPE[r - l];
	for (int i = 0; i < fanout; i++)
		for (int j = 0; j < this->actual_base_case; j++)
			pq.push({arr[l + i * run_size + j], 
				i * run_size + j});

	while (!pq.empty()) {
		auto curr = pq.top();
		pq.pop();
		tmp_arr[itr] = curr.first; itr++;

		if ((curr.second + 1) % this->actual_base_case == 0 &&
			(curr.second + 1 - l) % run_size != 0)
			for (TYPE i = 0; i < this->actual_base_case; i++)
				pq.push({arr[curr.second + i + 1], 
					curr.second + i + 1});
	}
	for (TYPE i = l; i < r; i++)
    	arr[i] = tmp_arr[i - l];
}

/* l is for left index and r is right index + 1 of the 
sub-array of arr to be sorted */
void Sorter::externalMergeSort (TYPE arr[], TYPE l, TYPE r) {
	if (r < l) {
		cout << "error: r < l" << endl;
		return;
	}
	if (r - l <= this->base_case) {
		this->actual_base_case = (r - l);
		std::sort (arr + l, arr + r);
	}
	else {
		TYPE run_size = (r - l) / this->fanout;
		for (int i = 0; i < this->fanout; ++i) {
      		externalMergeSort (arr, 
      			l + i * run_size, 
      			l + (i + 1) * run_size);
		}
		if (type_of_run == "constant") {
			// std::sort (arr + l, arr + r);
			Sorter::kWayMerge (arr, l, r);
		}
		else if (type_of_run == "adversarial") {
			TYPE increased_memory_bytes = this->memory_bytes + num_elements * sizeof(TYPE);
			CacheHelper::limit_memory(increased_memory_bytes, cgroup);
			std::ofstream out = memoryProfileSetup (type_of_run, this->num_elements, this->memory_bytes);
			double tmp_time = get_wall_time();
			out << tmp_time - this->start_time << " " << increased_memory_bytes << std::endl;
			Sorter::kWayMerge (arr, l, r);
			CacheHelper::limit_memory(this->memory_bytes, cgroup);      
			tmp_time = get_wall_time();
			out << tmp_time - this->start_time << " " << this->memory_bytes << std::endl;
		}
		else if (type_of_run == "benevolent") {
			TYPE decreased_memory_bytes = this->memory_bytes / 2;
			CacheHelper::limit_memory(decreased_memory_bytes, cgroup);
			std::ofstream out = memoryProfileSetup (type_of_run, this->num_elements, this->memory_bytes);
			double tmp_time = get_wall_time();
			out << tmp_time - this->start_time << " " << decreased_memory_bytes << std::endl;
			Sorter::kWayMerge (arr, l, r);
			CacheHelper::limit_memory(this->memory_bytes, cgroup);
			tmp_time = get_wall_time();
			out << tmp_time - this->start_time << " " << this->memory_bytes << std::endl;
		}
	}
}
