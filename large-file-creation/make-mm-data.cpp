#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "../CacheHelper.h"
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#define TYPE int
using namespace std;

unsigned long length = 0;
char* datafile;
/*
This function converts a matrix in row major layout to Z-Morton row major layout.
x is the output
xo is the input
no is the width of the entire matrix,
nn is the width of the matrix in the current recursive call
*/
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
	if ( n <= CacheHelper::MM_BASE_SIZE )
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

/*
This function converts a matrix in row major layout to Z-Morton column major layout.
x is the output
xo is the input
no is the width of the entire matrix,
nn is the width of the matrix in the current recursive call
*/
void conv_RM_2_ZM_CM( TYPE* x, TYPE* xo, int n, int no )
{
	if ( n <= CacheHelper::MM_BASE_SIZE )
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
  if (argc < 2){
    std::cout << "Insufficient arguments! Usage: mm_data <matrix_width>\n";
    exit(1);
  }
  length = std::stol(argv[1]);
  datafile = new char[strlen(argv[2]) + 1](); strncpy(datafile,argv[2],strlen(argv[2]));
  std::cout << "Generating matrices of size and converting to ZM: " << (int)length << "x" << (int)length << "\n";

  int fdout;
  if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
    printf ("can't create nullbytes for writing\n");
    return 0;
  }

	TYPE* dst;
  if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*5, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
       printf ("mmap error for output with code");
       return 0;
  }

  for (unsigned int i = 0; i < length*length; i++)
	{
		dst[i] = i;
		//std::cout << array[i] << " ";
	}

  conv_RM_2_ZM_RM(dst+length*length,dst,length,length);
  std::cout << "Second input array\n";
  conv_RM_2_ZM_CM(dst+2*length*length,dst,length,length);

  for (unsigned int i = 0 ; i < length*length; i++){
    dst[i] = 0;
  }

	for (unsigned int i = length*length*3 ; i < 5*length*length; i++){
		dst[i] = 0;
	}

  return 0;

}
