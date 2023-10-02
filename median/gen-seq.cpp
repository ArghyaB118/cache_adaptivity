#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sys/resource.h>
#include <sys/time.h>

char *sym = "ACGT";

void gen_seq( char *s, int n )
{
  printf( ">%s\n", s );
  
  int i = 0;
 
  srand48( time( NULL ) ); 
  
  while ( i < n )
    {
     printf( "%c", sym[ ( ( lrand48( ) >> 8 ) % 4 ) ]);
     
     i++;
     if ( i % 60 == 0 ) printf( "\n" );
     else if ( i % 10 == 0 ) printf( " " );
    }
    
  if ( n % 60 ) printf( "\n" );
}


int main( int argc, char *argv[ ] )
{
  int n;
  
  if ( argc < 2 )
    {
     printf( "\n\nError: specify seq-length seq-name ( optional )\n\n" );
     return 1;
    }
  else
    {
     n = atoi( argv[ 1 ] );  
     
     if ( n <= 0 ) 
       {
        printf( "\n\nError: invalid sequence length\n\n" );
        return 2;
       }

     if ( argc > 2 ) gen_seq( argv[ 2 ], n );
     else gen_seq( "no-name", n );
    }  
 
  return 0;
}
