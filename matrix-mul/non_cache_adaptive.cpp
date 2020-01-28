#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <array>
#include <vector>
#include <fcntl.h>
#include <ctime>
#include <memory>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <string>
#include <stdlib.h>
#include <unistd.h>

const int B = 64;
std::ofstream out;
std::clock_t start;
double duration;

#define TYPE int
const unsigned long length = 8192;
const bool mem_profile = true;
const int mem_profile_depth = 5;
const int progress_depth = 3;
char* cgroup_name;
long starting_memory = -1;
std::vector<long> io_stats = {0,0};

std::string exec(std::string cmd) {
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
//limits the memory, memory in bytes and
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

//x is output, xo is input
void conv_RM_2_ZM_RM( TYPE* x, TYPE* xo, int n, int no ){
	if ( n <= B )
	{
		for ( int i = 0; i < n; i++ )
		{
			for ( int j = 0; j < n; j++ )
				( *x++ ) = ( *xo++ );

			xo += ( no - n );
		}
	}
	else
	{
		int nn = ( n >> 1 );
		int nn2 = nn * nn;

		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;

		conv_RM_2_ZM_RM( x, xo, nn, no );
		conv_RM_2_ZM_RM( x+m12, xo + nn, nn, no );
		conv_RM_2_ZM_RM( x+m21, xo + nn * no, nn, no );
		conv_RM_2_ZM_RM( x+m22, xo + nn * no + nn, nn, no );
	}
}

//x is output, xo is input
void conv_ZM_RM_2_RM( TYPE* x, TYPE* xo, int n, int no )
{
	if ( n <= B )
	{
		for ( int i = 0; i < n; i++ )
		{
			for ( int j = 0; j < n; j++ )
				( *xo++ ) = ( *x++ );

			xo += ( no - n );
		}
	}
	else
	{
		int nn = ( n >> 1 );
		int nn2 = nn * nn;
		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;

		conv_ZM_RM_2_RM( x, xo, nn, no );
		conv_ZM_RM_2_RM( x+ m12, xo + nn, nn, no );
		conv_ZM_RM_2_RM( x+ m21, xo + nn * no, nn, no );
		conv_ZM_RM_2_RM( x+ m22, xo + nn * no + nn, nn, no );
	}
}

//x is output, y is auxiiliary memory, u and v are inputs
void mm( TYPE* x, TYPE* u, TYPE* v, TYPE* y, int n0, int n)
{
	if ( n <= B )
	{
		for ( int i = 0; i < n; i++ )
		{
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
	else
	{
		std::string depth_trace = "";
		int n3 = length;
		int limit = 0;
		while (n3 > n || n3 == 1){
			n3 >>= 1;
			depth_trace += " ";
			limit++;
		}
    if (limit < progress_depth){
      std::cout << depth_trace << "Running matrix multiply with depth: " << limit;
  		std::cout << " value of n: " << n << std::endl;
    }
		int nn = ( n >> 1 );
		int nn2 = nn * nn;

		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;

    int n2 = n0;
    TYPE* y2 = y;
    while (n2 > n){
      y2 += n2*n2;
      n2 >>= 1;
    }
    //cout << "y2-y in this call: " << y2-y << endl;
		mm( x + m11, u + m11, v + m11, y, n0, nn);
		mm( x + m12, u + m11, v + m12, y, n0, nn );
		mm( x + m21, u + m21, v + m11, y, n0, nn );
		mm( x + m22, u + m21, v + m12, y, n0, nn );

		mm( y2 + m11, u + m12, v + m21, y, n0, nn );
		mm( y2 + m12, u + m12, v + m22, y, n0, nn );
		mm( y2 + m21, u + m22, v + m21, y, n0, nn );
		mm( y2 + m22, u + m22, v + m22, y, n0, nn );

		if (mem_profile && limit < mem_profile_depth){
        //std::cout << "Depth: " << limit << std::endl;
        duration = (int)(1000.0 * (std::clock() - start) / CLOCKS_PER_SEC);
        //std::cout << "Duration: " << duration << std::endl;
        out << duration << " " << 3*n*n*4 << std::endl;
        //std::cout << "Writing the output \n";
				limit_memory(3*n*n*4+10000,cgroup_name);
        //std::cout << "Limited the memory" << std::endl;
		}

    for (int i = 0; i < n*n; i++){
      x[i] += y2[i];
			y2[i] = 0;
    }

    if (mem_profile && limit < mem_profile_depth){
        //std::cout << "Depth2: " << limit << std::endl;
        duration = (int)(1000.0 * (std::clock() - start) / CLOCKS_PER_SEC);
        //std::cout << "Duration: " << duration << std::endl;
        out << duration << " " << starting_memory << std::endl;
				limit_memory(starting_memory,cgroup_name);
		}

	}
}

//y is auxiliary memory
void mm_root(TYPE* x, TYPE* u, TYPE* v, TYPE* y, int n){
	std::cout << "Start of root call\n";
  int extra_memory = 0;
  int n2 = n;
  while (n2 > B){
    extra_memory += n2*n2;
    n2 >>= 1;
  }
  std::cout << "extra_memory " << extra_memory << "\n";
  for (int i = 0; i < extra_memory; i++){
    y[i] = 0;
  }
  print_io_data(io_stats, "Printing I/O statistics AFTER loading output matrix to cache @@@@@ \n");
	std::cout << "About to multiply\n";
  mm(x, u, v, y, n, n);
}

//x is output, xo is input
void conv_RM_2_ZM_CM( TYPE* x, TYPE* xo, int n, int no )
{
	if ( n <= B )
	{
		for ( int i = 0; i < n; i++ )
		{
			TYPE* xx = x + i;

			for ( int j = 0; j < n; j++ )
			{
				( *xx ) = ( *xo++ );
				xx += n;
			}


			xo += ( no - n );
		}
	}
	else
	{
		int nn = ( n >> 1 );
		int nn2 = nn * nn;
		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;

		conv_RM_2_ZM_CM( x, xo, nn, no );
		conv_RM_2_ZM_CM( x+ m12, xo + nn, nn, no );
		conv_RM_2_ZM_CM( x+ m21, xo + nn * no, nn, no );
		conv_RM_2_ZM_CM( x+ m22, xo + nn * no + nn, nn, no );
	}
}

int main(int argc, char *argv[]){
  TYPE* dst;


  if (argc < 3){
    std::cout << "Insufficient arguments! Usage: cgroup_cache_adaptive <memory_limit> <cgroup_name>\n";
    exit(1);
  }

  int fdout;

  if ((fdout = open ("nullbytes", O_RDWR, 0x0777 )) < 0){
    printf ("can't create nullbytes for writing\n");
    return 0;
  }

  starting_memory = std::stol(argv[1])*1024*1024;
  cgroup_name = new char[strlen(argv[2]) + 1]();
  strncpy(cgroup_name,argv[2],strlen(argv[2]));
  out = std::ofstream("mem_profile.txt", std::ofstream::out);
  //limit_memory(starting_memory+10000,argv[2]);

	std::cout << "Running cache_adaptive matrix multiply with matrices of size: " << (int)length << "x" << (int)length << "\n";
  print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");



if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*5, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
  //if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*5, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0)) == (TYPE*)MAP_FAILED)){
       printf ("mmap error for output with code");
       return 0;
   }

  //std::cout << "First input array\n";
	for (unsigned int i = 0; i < length*length; i++)
	{
		dst[i] = i;
		//std::cout << array[i] << " ";
	}
  print_io_data(io_stats, "Printing I/O statistics AFTER loading first matrix @@@@@\n");
  conv_RM_2_ZM_RM(dst+length*length,dst,length,length);
	print_io_data(io_stats, "Printing I/O statistics AFTER first conversion @@@@@\n");
	/*std::cout << "First input array in Z-MORTON\n";
	for (stxxl::uint64 i = 0; i < length*length; i++)
	{
		std::cout << array[i+length*length] << " ";
	}
	std::cout << std::endl;
*/
  std::cout << "Second input array\n";
  print_io_data(io_stats, "Printing I/O statistics AFTER loading fourth matrix @@@@@ \n");
  conv_RM_2_ZM_CM(dst+2*length*length,dst,length,length);
  print_io_data(io_stats, "Printing I/O statistics AFTER copying result of second conversion to cache @@@@@ \n");

  for (unsigned int i = 0 ; i < length*length; i++){
    dst[i] = 0;
  }
  print_io_data(io_stats, "Printing I/O statistics AFTER loading output matrix to cache @@@@@ \n");


	std::cout << "===========================================\n";

  //MODIFY MEMORY WITH CGROUP

  limit_memory(starting_memory+10000,argv[2]);

  double duration;
	start = std::clock();
	mm_root(dst,dst+length*length,dst+length*length*2,dst+length*length*3,length);

	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	std::cout << "===========================================\n";
	std::cout << "Total multiplication time: " << duration << "\n";
	std::cout << "===========================================\n";
  std::cout << "Data: " << (unsigned int)dst[length*length/2/2+length] << std::endl;
  print_io_data(io_stats, "Printing I/O statistics AFTER matrix multiplication @@@@@ \n");
	/*std::cout << "Result array\n";
  for (unsigned int i = 0 ; i < length*length; i++){
    std::cout << dst[i] << " ";
  }
  std::cout << std::endl;
  */
  delete cgroup_name;
  out.close();
  return 0;
}
