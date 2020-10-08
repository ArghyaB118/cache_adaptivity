#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>
#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <cstring>
#include <fstream>

using namespace std;
#define TYPE int

unsigned long data_in_MiB = 0;
const int B = 2;
char* datafile;

int main(int argc, char *argv[]){
  if (argc < 2){
    std::cout << "Insufficient arguments! Usage: ./executables/make-unsorted-data $data_size_run\n";
    exit(1);
  }
  data_in_MiB = std::stol(argv[1]);
  datafile = new char[strlen(argv[2]) + 1](); strncpy(datafile,argv[2],strlen(argv[2]));
  int fdout;
  if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
    printf ("can't create nullbytes for writing\n");
    return 0;
  }

	TYPE* dst;
  if (((dst = (TYPE*) mmap(0, data_in_MiB*1024*1024, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
       printf ("mmap error for output with code");
       return 0;
  }

  for (unsigned long int i = 0; i < (data_in_MiB/sizeof(TYPE))*1024*1024; i++)
	{
    srand((unsigned) time(0));
		dst[i] = rand() % 1000;
		//std::cout << array[i] << " ";
	}
  return 0;
}

