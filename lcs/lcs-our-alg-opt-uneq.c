/*
Our cache-oblivious LCS algorithm from SODA'06.
Last Update: Sep 04, 2006 ( Rezaul Alam Chowdhury, UT Austin )
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>

#define DEFAULT_BASE 32

#define MAX_ALPHABET_SIZE 256

#define SYMBOL_TYPE char

#define IDX( b, t ) ( MAX_N + b + t )
#define max( a, b ) ( ( a ) > ( b ) ) ? ( a ) : ( b )
#define min( a, b ) ( ( a ) < ( b ) ) ? ( a ) : ( b )

#define BIDX( j, i ) ( ( ( j ) << LOG_BASE_N ) + j + i )

int MAX_N;
int BASE_N;
int LOG_BASE_N;

SYMBOL_TYPE *X;
SYMBOL_TYPE *Y;
SYMBOL_TYPE *Z;

int nx, ny;

int xp, yp, zp;

char **XS;
char **YS;

int *nxs;
int *nys;

int *rlen;
int *buf_rlen;
int *buf_up;
int *buf_left;
int *buf_up_left;

int *blen;

struct rusage *ru;
int *zps;

char alpha[ MAX_ALPHABET_SIZE + 1 ];

char *fname1;
char *fname2;

void free_memory( int r )
{
  int i;

  if ( Z != NULL ) free( Z );

  if ( rlen != NULL ) free( rlen );
  
  if ( buf_rlen != NULL ) free( buf_rlen );

  if ( buf_up != NULL ) free( buf_up );
  if ( buf_left != NULL ) free( buf_left );
  if ( buf_up_left != NULL ) free( buf_up_left );

  if ( blen != NULL ) free( blen );

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


int myceil( double v )
{
  int i;

  i = ( ( int ) v );

  if ( v > i ) i++;

  return i;
}


int allocate_memory( int m, int n, int r, int b )
{
  int i, d, nn, mm;

  nn = 1;

  while ( nn < m ) nn <<= 1;

  mm = min( m, n );

  Z = ( SYMBOL_TYPE * ) malloc( ( mm + 2 ) * sizeof( SYMBOL_TYPE ) );

  rlen = ( int * ) malloc( ( 2 * nn + 1 ) * sizeof( int ) );

  mm = n * ( ( int ) ceil( ( m * 1.0 ) / n ) - 1 );
  if ( mm > 0 ) buf_rlen = ( int * ) malloc( ( mm ) * sizeof( int ) );

  buf_up = ( int * ) malloc( ( 3 * nn ) * sizeof( int ) );
  buf_left = ( int * ) malloc( ( 3 * nn ) * sizeof( int ) );
  buf_up_left = ( int * ) malloc( ( 3 * nn ) * sizeof( int ) );

  XS = ( char ** ) malloc( ( r ) * sizeof( char * ) );
  YS = ( char ** ) malloc( ( r ) * sizeof( char * ) );

  nxs = ( int * ) malloc( ( r ) * sizeof( int ) );
  nys = ( int * ) malloc( ( r ) * sizeof( int ) );

  blen = ( int * ) malloc( ( b + 1 ) * ( b + 1 ) * sizeof( int ) );

  ru = ( struct rusage * ) malloc( ( r + 1 ) * sizeof( struct rusage ) );
  zps = ( int * ) malloc( ( r ) * sizeof( int ) );

  if ( ( Z == NULL ) || ( rlen == NULL ) || ( ( mm > 0 ) && ( buf_rlen == NULL ) )
       || ( buf_up == NULL ) || ( buf_left == NULL ) || ( buf_up_left == NULL )
       || ( XS == NULL ) || ( YS == NULL ) 
       || ( nxs == NULL ) || ( nys == NULL )
       || ( blen == NULL ) || ( ru == NULL ) || ( zps == NULL ) )
   {
    printf( "\nError: memory allocation failed!\n\n" );
    free_memory( r );
    return 0;
   }

  for ( i = 0; i < r; i++ )
   {
    XS[ i ] = ( char * ) malloc( ( m + 2 ) * sizeof( char ) );
    YS[ i ] = ( char * ) malloc( ( n + 2 ) * sizeof( char ) );

    if ( ( XS[ i ] == NULL ) || ( YS[ i ] == NULL ) )
     {
      printf( "\nError: memory allocation failed!\n\n" );
      free_memory( r );
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


int read_data_sep( int r )
{
  int i;
  FILE *fp;

  if ( ( fp = fopen( fname1, "r" ) ) == NULL ) return 0;
  fscanf( fp, "%d\n", &i );
  for ( i = 0; i < r; i++ )
    {
     if ( fscanf( fp, "%s\n", XS[ i ] + 1 ) != 1 ) return 0;
     nxs[ i ] = strlen( XS[ i ] + 1 );
     printf( "|X| = %d\n", nxs[ i ] );
    }
  fclose( fp );

  if ( ( fp = fopen( fname2, "r" ) ) == NULL ) return 0;
  fscanf( fp, "%d\n", &i );
  for ( i = 0; i < r; i++ )
    {
     if ( fscanf( fp, "%s\n", YS[ i ] + 1 ) != 1 ) return 0;
     nys[ i ] = strlen( YS[ i ] + 1 );
     printf( "|Y| = %d\n", nys[ i ] );
    }
  fclose( fp );

  return 1;
}



int get_m_n_sep( int *m, int *n )
{
  FILE *fp;

  if ( ( fp = fopen( fname1, "r" ) ) == NULL ) return 0;
  if ( fscanf( fp, "%d", m ) != 1 ) return 0;
  fclose( fp );

  if ( ( fp = fopen( fname2, "r" ) ) == NULL ) return 0;
  if ( fscanf( fp, "%d", n ) != 1 ) return 0;
  fclose( fp );

  return 1;
}



void copy_seq( int j )
{
  int i;

  nx = nxs[ j ];
  ny = nys[ j ];

  X = XS[ j ];
  Y = YS[ j ];
}


int lcs_straight_triangle( int bi, int bj, int n )
{
  int i, j, k, l, lt, nn;

  if ( n <= BASE_N )
    {
      for ( k = 0; k < n; k++ )
	{
         i = min( bi + k, xp );
         j = bj + ( bi + k - i );
         lt = IDX( 0, i - j );
	 j = min( bj + k, yp );
	 i = bi + ( bj + k - j );
         for ( l = IDX( 0, i - j ); l <= lt; l += 2, i++, j-- )
           {
	    if ( X[ i ] == Y[ j ] )
	      rlen[ l ] = rlen[ l ] + 1;
            else
	      rlen[ l ] = max( rlen[ l - 1 ], rlen[ l + 1 ] );
           }
	}
    }
  else
    {
      nn = n >> 1;

      lcs_straight_triangle( bi, bj, nn );
      lcs_inverted_triangle( bi, bj, nn );
      if ( xp >= bi + nn ) lcs_straight_triangle( bi + nn, bj, nn );
      if ( yp >= bj + nn ) lcs_straight_triangle( bi, bj + nn, nn );
    }

  return 0;
}


int lcs_inverted_triangle( int bi, int bj, int n )
{
  int i, j, k, l, lt, nn;

  if ( n <= BASE_N )
    {
      for ( k = n - 2; k >= 0; k-- )
	{
	 i = min( bi - 1 + n, xp );
	 j = bj - 1 + n - k + ( ( bi - 1 + n ) - i );
	 lt = IDX( 0, i - j );
	 j = min( bj - 1 + n, yp );
	 i = bi - 1 + n - k + ( ( bj - 1 + n ) - j );
         for ( l = IDX( 0, i - j ); l <= lt; l += 2, i++, j-- )
           {
	    if ( X[ i ] == Y[ j ] )
	      rlen[ l ] = rlen[ l ] + 1;
            else
	      rlen[ l ] = max( rlen[ l - 1 ], rlen[ l + 1 ] );
           }
	}
    }
  else
    {
      nn = n >> 1;

      if ( xp >= bi + nn ) lcs_inverted_triangle( bi + nn, bj, nn );
      if ( yp >= bj + nn ) lcs_inverted_triangle( bi, bj + nn, nn );
      if ( ( xp >= bi + nn ) && ( yp >= bj + nn ) ) 
        {
         lcs_straight_triangle( nn + bi, nn + bj, nn );
         lcs_inverted_triangle( nn + bi, nn + bj, nn );
	}
    }
}


int rec_LCS( int bi, int bj, int n, int f )
{
  int i, j, k, mm, nn, b = bi - bj, sv;

  if ( n <= BASE_N )
    {
      mm = xp - bi + 1;
      nn = yp - bj + 1;

      for ( k = 0; k <= mm; k++ )
          blen[ BIDX( 0, k ) ] = rlen[ IDX( b, k ) ];

      for ( k = 0; k <= nn; k++ )
	  blen[ BIDX( k, 0 ) ] = rlen[ IDX( b, -k ) ];

      for ( j = 1; j <= nn; j++ )
	 for ( i = 1, k = BIDX( j, 1 ); i <= mm; i++, k++ )
	   {
       	    if ( X[ bi + i - 1 ] == Y[ bj + j - 1 ] )
	        blen[ k ] = blen[ k - BASE_N - 2 ] + 1;
            else
	        blen[ k ] = max( blen[ k - BASE_N - 1 ], blen[ k - 1 ] );
	   }

      while ( ( mm > 0 ) && ( nn > 0 ) )
	{
	  if ( X[ bi + mm - 1 ] == Y[ bj + nn - 1 ] )
	    {
      	      Z[ zp++ ] = X[ bi + mm - 1 ];
	      mm--; nn--;
	    }
	  else if ( blen[ BIDX( nn - 1, mm ) ] > blen[ BIDX( nn, mm - 1 ) ] ) nn--;
	       else mm--;
	}

      xp = mm + bi - 1;
      yp = nn + bj - 1; 
    }
  else
    {
      nn = n >> 1;

      if ( ( xp >= bi + nn ) || ( yp >= bj + nn ) )
        {
	  sv = 1;

          for ( k = -nn; k <= nn; k++ )
	     buf_up_left[ f + k + nn ] = rlen[ IDX( b, k ) ];

          lcs_straight_triangle( bi, bj, nn );
          lcs_inverted_triangle( bi, bj, nn );
	}
      else sv = 0;

      if ( ( xp >= bi + nn ) && ( yp >= bj + nn ) )
	{
          for ( k = -nn; k <= nn; k++ )
	     buf_left[ f + k + nn ] = rlen[ IDX( b - nn, k ) ];

          lcs_straight_triangle( bi, bj + nn, nn );
          lcs_inverted_triangle( bi, bj + nn, nn );

          for ( k = -nn; k <= nn; k++ )
	     buf_up[ f + k + nn ] = rlen[ IDX( b + nn, k ) ];

          lcs_straight_triangle( bi + nn, bj, nn );
          lcs_inverted_triangle( bi + nn, bj, nn );

          rec_LCS( bi + nn, bj + nn, nn, f + n + 1 );

          if ( xp >= bi + nn )
	    {
             for ( k = -nn; k <= nn; k++ )
               rlen[ IDX( b + nn, k ) ] = buf_up[ f + k + nn ];
	    }
	  else if ( yp >= bj + nn )
	         {
                  for ( k = -nn; k <= nn; k++ )
	             rlen[ IDX( b - nn, k ) ] = buf_left[ f + k + nn ];
	         }
	}

      if ( xp >= bi + nn )
	rec_LCS( bi + nn, bj, nn, f + n + 1 );
      else if ( yp >= bj + nn )
	rec_LCS( bi, bj + nn, nn, f + n + 1 );

      if ( ( xp >= bi ) && ( yp >= bj ) )
	{
       	  if ( sv )
	    {
             for ( k = -nn; k <= nn; k++ )
	        rlen[ IDX( b, k ) ] = buf_up_left[ f + k + nn ];
	    }

          rec_LCS( bi, bj, nn, f + n + 1 );
	}
    }
}


int rec_linear_LCS( int r, int n )
{
  int i, j, k, l, t;

  nx = nxs[ r ];
  ny = nys[ r ];

  X = XS[ r ];
  Y = YS[ r ];

  for ( j = -ny; j < nx; j++ )
    rlen[ IDX( 0, j ) ] = 0;

  xp = nx; yp = ny;
  zp = 0;

  rec_LCS( 1, 1, n, 0 );
}



int find_rec_LCS( void )
{
  int i, j;
  SYMBOL_TYPE s;

  Z[ zp ] = 0;

  for ( i = 0, j = zp - 1; i < j; i++, j-- )
    {
      s = Z[ i ];
      Z[ i ] = Z[ j ];
      Z[ j ] = s;
    }

  printf( "LCS Length = %d\n", zp );
  printf( "LCS = " );
  for ( i = 0; i < zp; i++ )
    printf( "%c", Z[ i ] );
  printf( "\n\n" );

  return zp;
}


int verify( void )
{
  int i, j;

  for ( i = j = 1; j <= zp ; j++, i++ )
    {
      while ( ( i <= nx ) && ( Z[ j - 1 ] != X[ i ] ) ) i++;
      if ( i > nx ) break;
    }

  if ( j == zp + 1 ) printf( "Found in X!!!\n\n" );
  else printf( "Not Found in X!!!\n\n" );

  for ( i = j = 1; j <= zp ; j++, i++ )
    {
      while ( ( i <= nx ) && ( Z[ j - 1 ] != Y[ i ] ) ) i++;
      if ( i > ny ) break;
    }

  if ( j == zp + 1 ) printf( "Found in Y!!!\n\n" );
  else printf( "Not Found in Y!!!\n\n" );
}


char *conv_sec( double t, char *st )
{
  int h, m, s;

  s = ( int ) floor( t );
  if ( t - s >= 0.5 ) s++;
  m = ( int ) floor( s / 60.0 );
  s -= m * 60;
  h = ( int ) floor( m / 60.0 );
  m -= h * 60;

  sprintf( st, "%dh %dm %ds", h, m, s );

  return st;
}



int main( int argc, char *argv[ ] )
{
  int i, l, m, n, nn, r, b, prn;
  double ut, st, tt;
  char str[ 50 ];

  printf( "Prog: %s\n\n", argv[ 0 ] );

  if ( argc < 3 )
     {
      printf( "\nError: not enough arguments!\n" );
      printf( "Specify: n ( = length of sequence ) and r ( = number of runs ).\n\n" );
      return 0;
     }

  n = atoi( argv[ 1 ] );
  if ( n == -1 )
     {
      fname1 = argv[ 2 ];
      fname2 = argv[ 3 ];
      b = 2;
     }
  else b = 0;
  r = atoi( argv[ b + 2 ] );
  m = n;

  if ( n == 0 )
    {
     if ( scanf( "%d %d\n\n", &m, &n ) != 2 ) 
	{
         printf( "\nError: cannot read sequence lengths!\n" );
         return 0;
	}
    }
  else if ( n == -1 )
         {
          if ( !get_m_n_sep( &m, &n ) ) 
             {
              printf( "\nError: cannot read sequence lengths!\n" );
              return 0;
	     }
         }

  if ( m < n ) 
    {
     printf( "\nError: m < n!\n" );
     return 0;
    }

  MAX_N = 1;
  while ( MAX_N < m ) MAX_N <<= 1;

  if ( argc > b + 3 ) 
    {
     BASE_N = atoi( argv[ b + 3 ] );
     if ( BASE_N <= 0 ) BASE_N = DEFAULT_BASE;
    }
  else BASE_N = DEFAULT_BASE;

  l = BASE_N;
  LOG_BASE_N = 0;
  while ( l > 1 )
    {
     l >>= 1;
     LOG_BASE_N++;
    }

  if ( argc > b + 4 ) prn = atoi( argv[ b + 4 ] );
  else prn = 0;
    
  if ( !allocate_memory( m, n, r, BASE_N )) return 0;

  if ( b == 0 )
    {
     if ( !read_data( r ) )
       {
        printf( "\nError: failed to read data!\n\n" );
        free_memory( r );
        return 0;
       }
    }
  else
    {
     if ( !read_data_sep( r ) )
       {
        printf( "\nError: failed to read data!\n\n" );
        free_memory( r );
        return 0;
       }
    }

  printf( "\ndata read for m = %d and n = %d\n", m, n );
  printf( "number of runs = %d, base case = %d\n\n", r, BASE_N );

  getrusage( RUSAGE_SELF, &ru[ 0 ] );

  for ( i = 0; i < r; i++ )
     {
      rec_linear_LCS( i, MAX_N );
      zps[ i ] = zp;
      if ( prn )
        {
         find_rec_LCS( );
         verify( );
        }
      getrusage( RUSAGE_SELF, &ru[ i + 1 ] );
     }

  ut =  ru[ r ].ru_utime.tv_sec + ( ru[ r ].ru_utime.tv_usec * 0.000001 )
      - ( ru[ 0 ].ru_utime.tv_sec + ( ru[ 0 ].ru_utime.tv_usec * 0.000001 ) );
  st =  ru[ r ].ru_stime.tv_sec + ( ru[ r ].ru_stime.tv_usec * 0.000001 )
      - ( ru[ 0 ].ru_stime.tv_sec + ( ru[ 0 ].ru_stime.tv_usec * 0.000001 ) );
  tt = ut + st;

  printf( "rec_linear_LCS ( dynamic mem ) user time = %.4lf sec, system time = %.4lf sec, total time = %.4lf\n\n",
	  ut, st, tt );

  printf( "rec_linear_LCS ( dynamic mem ) avg. user time = %.4lf sec, avg. system time = %.4lf sec, avg. total time = %.4lf ( %s )\n\n",
         ut / r, st / r, tt / r, conv_sec( tt / r, str ) );

  for ( i = 0; i < r; i++ )
    {
     ut =  ru[ i + 1 ].ru_utime.tv_sec + ( ru[ i + 1 ].ru_utime.tv_usec * 0.000001 )
        - ( ru[ i ].ru_utime.tv_sec + ( ru[ i ].ru_utime.tv_usec * 0.000001 ) );
     st =  ru[ i + 1 ].ru_stime.tv_sec + ( ru[ i + 1 ].ru_stime.tv_usec * 0.000001 )
        - ( ru[ i ].ru_stime.tv_sec + ( ru[ i ].ru_stime.tv_usec * 0.000001 ) );
     tt = ut + st;

     printf( "run %d: user time = %.4lf sec, system time = %.4lf sec, total time = %.4lf ( %s ), LCS length = %d\n",
	     i + 1, ut, st, tt, conv_sec( tt, str ), zps[ i ] );
    }

  printf( "\n" );

  free_memory( r );

  return 1;
}
