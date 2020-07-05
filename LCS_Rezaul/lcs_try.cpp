//we assume both the strings are of length n
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>
using namespace std;

#define SYMBOL_TYPE char
SYMBOL_TYPE *X;
SYMBOL_TYPE *Y;
SYMBOL_TYPE *Z;
char **XS, **YS;
int *nxs, *nys, *zps, **len;
struct rusage *ru;

void free_memory(int n) {
	if ( Z != NULL ) free(Z);
	if ( len != NULL) {
		for (int i = 0; i <= n; ++i)
			if ( len[ i ] != NULL ) free(len[i]);
		free(n);
	}
	if ( XS != NULL ) free(XS);
	if ( YS != NULL ) free(YS);
	if ( nxs != NULL ) free(nxs);
  	if ( nys != NULL ) free(nys);
  	if ( ru != NULL ) free(ru);
	if ( zps != NULL ) free(zps);
}

int allocate_memory (int m, int n) {
	Z = (SYMBOL_TYPE * ) malloc((n + 2 ) * sizeof(SYMBOL_TYPE));
	XS = (char **) malloc(sizeof(char *));
	YS = (char **) malloc(sizeof(char *));
	//Z = new (SYMBOL_TYPE * )[(n + 2 ) * sizeof(SYMBOL_TYPE)];
	//XS = new (char **) 
	//nxs = ( int * ) malloc( ( r ) * sizeof( int ) );
	nxs = new int[sizeof(int)];
	nys = new int[sizeof(int)];
	len = (int **) malloc((n + 1) * sizeof(int *));

	ru = (struct rusage *) malloc(sizeof(struct rusage));
  	zps = new int[sizeof(int)];
}

int main(int argc, char *argv[]) {
	int m,n;
	if (argc < 2)
		cout << "please enter: ./a.out $data_size" << endl;
	n = atoi(argv[1]);

	if (!allocate_memory(m, n)) return 0;
}