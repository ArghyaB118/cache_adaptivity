/*
Iterative LCS.
Last Update: June 28, 2005 ( Rezaul Alam Chowdhury, UT Austin )
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>

#define MAX_ALPHABET_SIZE 256

#define SYMBOL_TYPE char

#define max( a, b ) ( ( a ) > ( b ) ) ? ( a ) : ( b )
#define min( a, b ) ( ( a ) < ( b ) ) ? ( a ) : ( b )

//#define CLOCK ( ( double ) clock( ) )

SYMBOL_TYPE *X;
SYMBOL_TYPE *Y;
SYMBOL_TYPE *Z;

int nx, ny;

int xp, yp, zp;

char **XS;
char **YS;

int *nxs;
int *nys;

int **len;

struct rusage *ru;
int *zps;

char alpha[ MAX_ALPHABET_SIZE + 1 ];

//double tmp_t, copy_t, base_t, tri_base_t;


void free_memory( int r, int n )
{
  int i;

  if ( Z != NULL ) free( Z );

  if ( len != NULL )
     {
      for ( i = 0; i <= n; i++ )
        if ( len[ i ] != NULL ) free( len[ i ] );

      free( len );
     }

  if ( XS != NULL )
    {
     for ( i = 0; i < r; i++ )
       if ( XS[ i ] != NULL ) free( XS[ i ] );

     free( XS );
    }

  if ( YS != NULL )
    {
     for ( i = 0; i < r; i++ )
       if ( YS[ i ] != NULL ) free( YS[ i ] );

     free( YS );
    }

  if ( nxs != NULL ) free( nxs );
  if ( nys != NULL ) free( nys );

  if ( ru != NULL ) free( ru );

  if ( zps != NULL ) free( zps );
}


int allocate_memory( int m, int n, int r )
{
  int i, d, mm;

  mm = min( m, n );

  Z = ( SYMBOL_TYPE * ) malloc( ( mm + 2 ) * sizeof( SYMBOL_TYPE ) );

  XS = ( char ** ) malloc( ( r ) * sizeof( char * ) );
  YS = ( char ** ) malloc( ( r ) * sizeof( char * ) );

  nxs = ( int * ) malloc( ( r ) * sizeof( int ) );
  nys = ( int * ) malloc( ( r ) * sizeof( int ) );

  len = ( int **) malloc( ( n + 1 ) * sizeof( int * ) );

  ru = ( struct rusage * ) malloc( ( r + 1 ) * sizeof( struct rusage ) );
  zps = ( int * ) malloc( ( r ) * sizeof( int ) );

  if ( ( Z == NULL ) || ( XS == NULL ) || ( YS == NULL )
       || ( nxs == NULL ) || ( nys == NULL )
       || ( ru == NULL ) || ( zps == NULL ) )
   {
    printf( "\nError: memory allocation failed!\n\n" );
    free_memory( r, n );
    return 0;
   }

  for ( i = 0; i < r; i++ )
   {
    XS[ i ] = ( char * ) malloc( ( m + 2 ) * sizeof( char ) );
    YS[ i ] = ( char * ) malloc( ( n + 2 ) * sizeof( char ) );

    if ( ( XS[ i ] == NULL ) || ( YS[ i ] == NULL ) )
     {
      printf( "\nError: memory allocation failed!\n\n" );
      free_memory( r, n );
      return 0;
     }
   }

  for ( i = 0; i <= n; i++ )
   {
    len[ i ] = ( int * ) malloc( ( m + 1 ) * sizeof( int ) );

    if ( len[ i ] == NULL )
     {
      printf( "\nError: memory allocation failed!\n\n" );
      free_memory( r, n );
      return 0;
     }
   }

  return 1;
}


int read_data( int r )
{
  int i, d;

  scanf( "alphabet: %s\n\n", alpha );

  for ( i = 0; i < r; i++ )
   {
    if ( scanf( "sequence pair %d:\n\n", &d ) != 1 ) return 0;
    printf( "sequence pair %d:\n", d );
    if ( scanf( "X = %s\n", XS[ i ] + 1 ) != 1 ) return 0;
    nxs[ i ] = strlen( XS[ i ] + 1 );
    printf( "|X| = %d\n", nxs[ i ] );
    if ( scanf( "Y = %s\n\n", YS[ i ] + 1 ) != 1 ) return 0;
    nys[ i ] = strlen( YS[ i ] + 1 );
    printf( "|Y| = %d\n\n", nys[ i ] );
   }

  return 1;
}



int LCS_classic( int r )
{
  int i, j, m, n;

  m = nxs[ r ];
  n = nys[ r ];

  X = XS[ r ];
  Y = YS[ r ];

  for ( i = 0; i <= m; i++ )
     len[ 0 ][ i ] = 0;

  for ( j = 0; j <= n; j++ )
     len[ j ][ 0 ] = 0;

  for ( j = 1; j <= n; j++ )
    for ( i = 1; i <= m; i++ )
      {
	if ( X[ i ] == Y[ j ] )
          {
	    len[ j ][ i ] = len[ j - 1 ][ i - 1 ] + 1;
          }
        else
          {
	    if (len[ j - 1 ][ i ] > len[ j ][ i - 1 ])
	      {
		len[ j ][ i ] = len[ j - 1 ][ i ];
	      }
	    else
	      {
		len[ j ][ i ] = len[ j ][ i - 1 ];
	      }
          }
      }

  return len[ n ][ m ];
}


int main( int argc, char *argv[ ] )
{
  int i, l, m, n, nn, r;
  //  time_t t1, t2;
  //  double c1, c2, tck;
  double ut, st, tt;

  printf( "Prog: %s\n\n", argv[ 0 ] );

  if ( argc < 3 )
     {
      printf( "\nError: not enough arguments!\n" );
      printf( "Specify: n ( = length of sequence ) and r ( = number of runs ).\n\n" );
      return 0;
     }

  n = atoi( argv[ 1 ] );
  r = atoi( argv[ 2 ] );
  m = n;

  if ( n <= 0 )
     printf("%d\n", n);
     if ( scanf( "%d %d\n\n", &m, &n ) != 2 )
	{
         printf( "\nError: cannot read sequence lengths!\n" );
         return 0;
	}

  if ( m < n )
    {
     printf( "\nError: m < n!\n" );
     return 0;
    }

  if ( !allocate_memory( m, n, r )) return 0;

  if ( !read_data( r ) )
    {
     printf( "\nError: failed to read data!\n\n" );
     free_memory( r, n );
     return 0;
    }

  printf( "\ndata read for m = %d and n = %d\n", m, n );
  printf( "number of runs = %d\n\n", r );

  //  t1 = time( NULL );
  //  copy_t = base_t = tri_base_t = 0;
  //  c1 = CLOCK;
  getrusage( RUSAGE_SELF, &ru[ 0 ] );

  for ( i = 0; i < r; i++ )
     {
      zps[ i ] = LCS_classic( i );
      getrusage( RUSAGE_SELF, &ru[ i + 1 ] );
     }

  //  c2 = CLOCK;
  //  tck = c2 - c1;
  ut =  ru[ r ].ru_utime.tv_sec + ( ru[ r ].ru_utime.tv_usec * 0.000001 )
      - ( ru[ 0 ].ru_utime.tv_sec + ( ru[ 0 ].ru_utime.tv_usec * 0.000001 ) );
  st =  ru[ r ].ru_stime.tv_sec + ( ru[ r ].ru_stime.tv_usec * 0.000001 )
      - ( ru[ 0 ].ru_stime.tv_sec + ( ru[ 0 ].ru_stime.tv_usec * 0.000001 ) );
  tt = ut + st;

//  t2 = time( NULL );
//  printf( "rec_linear_LCS ( dynamic mem ) time = %.4lf sec ( %.0lf ticks ), Average time = %lf sec\n\n",
//	  tck / CLOCKS_PER_SEC, tck,  tck / ( 1.0 * r * CLOCKS_PER_SEC ));

//  printf( "copy ticks = %.0lf, base ticks = %.0lf, triangle base ticks = %.0lf\n\n",
//           copy_t, base_t, tri_base_t );

  printf( "LCS_classic ( dynamic mem ) user time = %.4lf sec, system time = %.4lf sec, total time = %.4lf\n\n",
	  ut, st, tt );

  printf( "LCS_classic ( dynamic mem ) avg. user time = %.4lf sec, avg. system time = %.4lf sec, avg. total time = %.4lf\n\n",
         ut / r, st / r, tt / r );

  for ( i = 0; i < r; i++ )
    {
     ut =  ru[ i + 1 ].ru_utime.tv_sec + ( ru[ i + 1 ].ru_utime.tv_usec * 0.000001 )
        - ( ru[ i ].ru_utime.tv_sec + ( ru[ i ].ru_utime.tv_usec * 0.000001 ) );
     st =  ru[ i + 1 ].ru_stime.tv_sec + ( ru[ i + 1 ].ru_stime.tv_usec * 0.000001 )
        - ( ru[ i ].ru_stime.tv_sec + ( ru[ i ].ru_stime.tv_usec * 0.000001 ) );
     tt = ut + st;

     printf( "run %d: user time = %.4lf sec, system time = %.4lf sec, total time = %.4lf, LCS length = %d\n",
	     i + 1, ut, st, tt, zps[ i ] );
    }

  printf( "\n" );

  free_memory( r, n );
//  while (!getchar());

  return 1;
}
