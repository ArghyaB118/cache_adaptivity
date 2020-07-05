#include "../CacheHelper.h"
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
#include <cstring>
#include <fcntl.h>
#include <queue>
#include <algorithm>
#include <fstream>
#include <time.h>
#include <stdio.h>
using namespace std;
#define TYPE int

int main(int argc, char *argv[]) {
	int ipcfd;
	int* dst2, dst3;
	if ((ipcfd = open ("balloon_data/IPCTEST", O_RDWR, 0x0777 )) < 0){
		printf ("can't create file for writing\n");
		return 0;
  }
	if (((dst2 = (int*) mmap(0, 10*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED , ipcfd, 0)) == (int*)MAP_FAILED)){
     printf ("mmap error for output with code");
     return 0;
	}
	for (int i = 0; i < 10; i++)
		dst2[i] = i;
  munmap(dst2, 10*sizeof(int));

  if (((dst3 = (int*) mmap(0, 10*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED , ipcfd, 0)) == (int*)MAP_FAILED)){
     printf ("mmap error for output with code");
     return 0;
  }
	for (int i = 0; i < 10; i++)
		cout << dst3[i] << endl;

}