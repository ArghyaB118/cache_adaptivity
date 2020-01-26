#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>
#include <array>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <memory>
#include <string>
#include <stdlib.h>
#include <unistd.h>
const int B = 64;

#define TYPE int
const unsigned long length = 8192;
const int progress_depth = 3;

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


//x is the output, xo is the input
void conv_RM_2_ZM_RM(TYPE* x,TYPE* xo, int n,int no ){
  /*
	std::string depth_trace = "";
	int n3 = length;
	int limit = 0;
	while (n3 > n || n3 == 1){
		n3 /= 2;
		depth_trace += " ";
		limit++;
	}
	std::cout << depth_trace << "Running conv with depth: " << limit;
	std::cout << " value of n: " << n << std::endl;
  */
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

//x is the output, u and v are the two inputs
void mm( TYPE* x, TYPE* u, TYPE* v, int n )
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
			n3 /= 2;
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
		mm( x + m11, u + m11, v + m11, nn );
		mm( x + m12, u + m11, v + m12, nn );
		mm( x + m21, u + m21, v + m11, nn );
		mm( x + m22, u + m21, v + m12, nn );

		mm( x + m11, u + m12, v + m21, nn );
		mm( x + m12, u + m12, v + m22, nn );
		mm( x + m21, u + m22, v + m21, nn );
		mm( x + m22, u + m22, v + m22, nn );
	}
}

//x is the output, xo is the input
void conv_RM_2_ZM_CM( TYPE* x, TYPE* xo, int n, int no )
{
	if ( n <= B )
	{
		for ( int i = 0; i < n; i++ )
		{
			TYPE* xx = x + i;

			for ( int j = 0; j < n; j++ )
			{
				(*xx ) = ( *xo++ );
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
	std::cout << "Running cache_adaptive matrix multiply with matrices of size: " << (int)length << "x" << (int)length << "\n";
  std::vector<long> io_stats = {0,0};
  print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");

  int fdout;

  if ((fdout = open ("nullbytes", O_RDWR, 0x0777 )) < 0){
    printf ("can't create nullbytes for writing\n");
    return 0;
  }

  if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*3, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
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
  limit_memory(std::stol(argv[1])*1024*1024,argv[2]);

  std::clock_t start;
  double duration;
	start = std::clock();
	mm(dst,dst+length*length,dst+length*length*2,length);

	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

	std::cout << "===========================================\n";
	std::cout << "Total multiplication time: " << duration << "\n";
	std::cout << "===========================================\n";
  std::cout << "Data: " << (unsigned int)dst[length*length/2/2+length] << std::endl;
	std::cout << "===========================================\n";
	std::cout << "===========================================\n";
  print_io_data(io_stats, "Printing I/O statistics AFTER matrix multiplication @@@@@ \n");
	/*std::cout << "Result array\n";
  for (unsigned int i = 0 ; i < length*length; i++){
    std::cout << dst[i] << " ";
  }
  std::cout << std::endl;
  */
  return 0;
}
