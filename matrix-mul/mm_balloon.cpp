#include "../CacheHelper.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <chrono>
#include <fstream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <thread>

#define TYPE int
using namespace std;

unsigned long long memory;
unsigned long length = 0;
time_t start, finish; // introduced to measure wall clock time
double duration;
std::ofstream mm_out, out;
std::string type_of_run;
TYPE* dst;
unsigned long long* dst2;




void scan_add( TYPE* x, TYPE* y, int n ) {
	for (int i = 0; i < n * n; i++) {
		x[i] += y[i];
	}
}


//x is the output, u and v are the two inputs
void mm( TYPE* x, TYPE* u, TYPE* v, int n ) {
	for ( int i = 0; i < n; i++ ) {
		TYPE* vv = v;
		for ( int j = 0; j < n; j++ )
		{
			TYPE t = 0;

			for ( int k = 0; k < n; k++ )
				t += u[ k ] * vv[ k ];

			( *x++ ) += t;
			vv += n;
		}
		u += n;
	}	
}


void mm_scan( TYPE* x, TYPE* u, TYPE* v, TYPE* y, int n0, int n) {
	if ( n <= CacheHelper::MM_BASE_SIZE ) {
		mm( x , u , v, n );
	}
	else {
	    int nn = ( n >> 1 );
		int nn2 = nn * nn;

		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;

	    int n2 = n0;
	    TYPE* y2 = y;
	    while (n2 > n){
	      y2 += n2 * n2;
	      n2 >>= 1;
	    }
	    
	    mm_scan( x + m11, u + m11, v + m11, y, n0, nn );
		mm_scan( x + m12, u + m11, v + m12, y, n0, nn );
		mm_scan( x + m21, u + m21, v + m11, y, n0, nn );
		mm_scan( x + m22, u + m21, v + m12, y, n0, nn );

		mm_scan( y2 + m11, u + m12, v + m21, y, n0, nn );
		mm_scan( y2 + m12, u + m12, v + m22, y, n0, nn );
		mm_scan( y2 + m21, u + m22, v + m21, y, n0, nn );
		mm_scan( y2 + m22, u + m22, v + m22, y, n0, nn );

		if (type_of_run == "constant") 
			scan_add( x , y2, n );
		else if (type_of_run == "adversarial") {
			duration = time(NULL) - start;
      		out << duration << " " << memory + 5*n*n*sizeof(TYPE) << std::endl;
      		dst[0] = memory + 5 * n * n; // CacheHelper::limit_memory(memory, cgroup);
      		scan_add( x , y2, n );
      		duration = time(NULL) - start;
      		out << duration << " " << memory << std::endl;
      		dst[0] = memory; // CacheHelper::limit_memory(memory, cgroup);
		}
		else if (type_of_run == "benevolent") {
			duration = time(NULL) - start;
      		out << duration << " " << memory / 2 << std::endl;
      		dst[0] = memory / 2; // CacheHelper::limit_memory(memory, cgroup);
      		scan_add( x , y2, n );
      		duration = time(NULL) - start;
      		out << duration << " " << memory << std::endl;
      		dst[0] = memory; // CacheHelper::limit_memory(memory, cgroup);
		}
	}
}



//x is the output, u and v are the two inputs
void mm_inplace( TYPE* x, TYPE* u, TYPE* v, int n ) {
	if ( n <= CacheHelper::MM_BASE_SIZE ) {
		mm( x , u , v, n );
	}
	else {
		int nn = ( n >> 1 );
		int nn2 = nn * nn;

		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;
		
		mm_inplace( x + m11, u + m11, v + m11, nn );
		mm_inplace( x + m12, u + m11, v + m12, nn );
		mm_inplace( x + m21, u + m21, v + m11, nn );
		mm_inplace( x + m22, u + m21, v + m12, nn );

		mm_inplace( x + m11, u + m12, v + m21, nn );
		mm_inplace( x + m12, u + m12, v + m22, nn );
		mm_inplace( x + m21, u + m22, v + m21, nn );
		mm_inplace( x + m22, u + m22, v + m22, nn );
	}
}


void mm_block( TYPE* x, TYPE* u, TYPE* v, int n ) {
	if ( n <= CacheHelper::MM_BLOCK_BASE_SIZE ) {
		mm( x , u , v , n );
	}
	else {
		int nn = CacheHelper::MM_BLOCK_BASE_SIZE;
		int nn2 = nn * nn;
		for ( int i = 0; i < n / nn ; i++ )
			for ( int j = 0; j < n / nn ; j++ )
				for ( int k = 0; k < n / nn ; k++ )
					mm( x + (n * i / nn + j) * nn2, u + (n * i / nn + k) * nn2, v + (n * j / nn + k) * nn2 , nn );
	}
}

int memProfSetup() {
  int ipcfd;
  if ((ipcfd = open ("balloon_data/IPCTEST", O_RDWR, 0x0777 )) < 0){
    printf ("can't create file for writing\n");
    return -1;
  }
  if (((dst2 = (unsigned long long*) mmap(0, sizeof(unsigned long long), PROT_READ | PROT_WRITE, MAP_SHARED , ipcfd, 0)) == (unsigned long long*)MAP_FAILED)){
       printf ("mmap error for output with code");
       return -1;
  }

  if (type_of_run == "adversarial") {
    std::string temp = "mem_profiles/mm_profile_adversarial_" + std::to_string(length) + ".txt";
    out = std::ofstream(temp, std::ofstream::out);
  }
  else if (type_of_run == "benevolent") {
    std::string temp = "mem_profiles/mm_profile_benevolent_" + std::to_string(length) + ".txt";
    out = std::ofstream(temp, std::ofstream::out);
  }
  return 1;
}

int main(int argc, char *argv[]){
	if (argc < 4){
		std::cout << "cgexec -g memory:cache-test-arghya ./executables/parallel_mm <0=MM-INPLACE/1=MM-SCAN> matrix-width memory-size-MiB data_files/nullbytes <cgroup_name> <num_threads(1/4)>\n";
		exit(1);
	}
	std::ofstream mm_out = std::ofstream("out-mm.txt",std::ofstream::out | std::ofstream::app);
	std::string program = argv[1]; // "mm_block" or "mm_inplace" or "mm_scan"
	length = std::stol(argv[2]); memory = 1048576ULL * std::stoi(argv[3]); 
	type_of_run = argv[4];
	std::string datafile = argv[5];
	//cgroup_name = new char[strlen(argv[5]) + 1](); strncpy(cgroup_name,argv[5],strlen(argv[5]));

	std::cout << "Running cache_adaptive matrix multiply with matrices of size: " << (int)length << "x" << (int)length << std::endl;
	std::vector<long> io_stats = {0,0};
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");

	int fdout;	
	if ((fdout = open (datafile.c_str(), O_RDWR, 0x0777 )) < 0){
		printf ("can't create nullbytes for writing\n");
		return 0;
	}

	if (program == "mm_block" || program == "mm_inplace") {
		if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*3, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
		   printf ("mmap error for output with code");
		   return 0;
		}
	}
	else if (program == "mm_scan") {
		if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*5, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
		   printf ("mmap error for output with code");
		   return 0;
		}
	}
	else {
		cout << "program can be only mm_scan, mm_inplace, or mm_block" << endl;
	}
	if (program == "mm_scan") {
		if (type_of_run == "adversarial" || type_of_run == "benevolent") {
		    int temp = memProfSetup();
		    if (temp == -1)
		      return 0;
	  	}
	}
	dst[0] = memory; out << "0 " << memory << std::endl;	
	std::chrono::system_clock::time_point t_start = std::chrono::system_clock::now();
//	std::clock_t start = std::clock();
	start = time(NULL); 
	if (program == "mm_scan")
		mm_scan( dst, dst + length*length, dst + length*length*2, dst + length*length*3, length, length );
	else if (program == "mm_inplace")
		mm_inplace( dst, dst + length*length, dst + length*length*2, length );
	else if (program == "mm_block")
		mm_block( dst, dst + length*length, dst + length*length*2, length );
	else
		return -1;
	
	std::chrono::system_clock::time_point t_end = std::chrono::system_clock::now();
//	double cpu_time = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	auto wall_time = std::chrono::duration<double, std::milli>(t_end-t_start).count();
	finish = time(NULL);
  	duration = (finish - start);

	std::cout << "Total wall time: " << wall_time << " " << duration << std::endl;
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics AFTER matrix multiplication @@@@@ \n");
	mm_out << program << "," << datafile.substr(11,21) << "," << length << "," << duration << "," << wall_time << "," << (float)io_stats[0]/1000000.0 << "," << (float)io_stats[1]/1000000.0 << "," << (float)(io_stats[0] + io_stats[1])/1000000.0 << std::endl;
  	mm_out.close();
  	return 0;
}
