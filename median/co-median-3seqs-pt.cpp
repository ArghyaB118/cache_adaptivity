#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <locale>
#include <vector>
#include <sys/resource.h>
#include <sys/time.h>

#define GTOD_TIMER

#include "timers.c"

#include "med-3seqs-pt.h"

#define MAX_ID_LENGTH         100

#ifndef MAX_SEQ_LENGTH
  #define MAX_SEQ_LENGTH  1000000
#endif

#ifndef DEFAULT_GAP_OPEN_COST
  #define DEFAULT_GAP_OPEN_COST 2
#endif

#ifndef DEFAULT_GAP_EXTENSION_COST
  #define DEFAULT_GAP_EXTENSION_COST 1
#endif

#ifndef DEFAULT_MISMATCH_COST
  #define DEFAULT_MISMATCH_COST 1
#endif

#ifndef DEFAULT_BASE_SIZE
  #define DEFAULT_BASE_SIZE 64
#endif

#ifndef MAX_CONCURRENT_THREADS
  #define MAX_CONCURRENT_THREADS 16
#endif

enum file_formats{ FASTA, PHYLIP_SEQUENTIAL, PHYLIP_INTERLEAVED };

#ifndef DEFAULT_FILE_FORMAT
  #define DEFAULT_FILE_FORMAT  FASTA
#endif


int nx, ny, nz;

char X[ MAX_SEQ_LENGTH + 2 ];
char Y[ MAX_SEQ_LENGTH + 2 ];
char Z[ MAX_SEQ_LENGTH + 2 ];

vector<char> XX;
vector<char> YY;
vector<char> ZZ;
vector<char> Med;

char Xid[ MAX_ID_LENGTH + 2 ];
char Yid[ MAX_ID_LENGTH + 2 ];
char Zid[ MAX_ID_LENGTH + 2 ];
char Medid[ MAX_ID_LENGTH + 2 ] = "median";

std::vector< struct rusage > ru;

char *SYM = (char*)"ACGT";

enum err_msgs{ SEQUENCE_READ, LENGTH_READ, NO_SEQUENCE, SEQUENCE_TOO_LONG, INVALID_SEQUENCE_LENGTH };


int read_next_seq_FASTA( FILE *fp, char *sid, char *sq, int &len )
{
  int c, i;

  while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '>' ) );
   
  if ( c != '>' ) return NO_SEQUENCE;

  while ( ( ( c = fgetc( fp ) ) != EOF ) && ( isspace( c ) || iscntrl( c ) ) );

  ungetc( c, fp );

  i = 0;

  while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) )
    {
     if ( ( i < MAX_ID_LENGTH ) && !iscntrl( c ) ) sid[ i++ ] = c;
    }

  sid[ i ] = 0;

  if ( c == EOF ) return NO_SEQUENCE;

  if ( c != '\n' )
    {
     while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) );
     if ( c == EOF ) return NO_SEQUENCE;
    }

  i = 0;

  while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '>' ) )
    {
     if ( isspace( c ) || ( c == '\n' ) ) continue;
     c = toupper( c );
     if ( strchr( SYM, c ) == NULL ) continue;
     if ( i < MAX_SEQ_LENGTH ) sq[ i++ ] = c;
     else return SEQUENCE_TOO_LONG;
    }

  sq[ i ] = 0;

  len = i;

  if ( c == '>' ) ungetc( c, fp );

  return SEQUENCE_READ;
}


int read_sequences_FASTA( char *fn )
{
  FILE *fp;

  if ( fn[ 0 ] )
    {
     if ( ( fp = fopen( fn, "rt" ) ) == NULL )
       {
        printf( "Error: Unable to open input file ( %s )!\n\n", fn );
        return 0;
       }
    }
  else fp = stdin;

  int l;

  for ( int i = 0; i < 3; i++ )
    {
     if ( i == 0 ) l = read_next_seq_FASTA( fp, Xid, X, nx );
     else if (i == 1) l = read_next_seq_FASTA( fp, Yid, Y, ny );
     else l = read_next_seq_FASTA( fp, Zid, Z, nz );

     if ( l == SEQUENCE_TOO_LONG )
       {
        printf( "Error: Sequence too long ( > %d )! Please recompile with MAX_SEQ_LENGTH redefined ( i.e., with -DMAX_SEQ_LENGTH=new-length )!\n\n", MAX_SEQ_LENGTH );
        return 0;
       }

     if ( l == NO_SEQUENCE )
       {
        printf( "Error: Failed to read sequence %d!\n\n", i + 1 );
        return 0;
       }
    }

  if ( fn[ 0 ] ) fclose( fp );

  return 1;
}


int read_next_seq_PHYLIP_SEQUENTIAL( FILE *fp, char *sid, char *sq, int &len, int mlen )
{
  int c, i, j;

  i = 0;
  while ( ( ( c = fgetc( fp ) ) != EOF ) && ( i < 10 ) ) sid[ i++ ] = c;
  sid[ i ] = 0;
   
  if ( c == EOF ) return NO_SEQUENCE;

  i = j = 0;
  while ( ( ( c = fgetc( fp ) ) != EOF ) && ( j < mlen ) )
    {
     if ( isspace( c ) || isdigit( c ) || ( c == '\n' ) || ( c == '.' ) ) continue;
     j++;
     c = toupper( c );     
     if ( strchr( SYM, c ) == NULL ) continue;
     if ( i < MAX_SEQ_LENGTH ) sq[ i++ ] = c;
     else return SEQUENCE_TOO_LONG;
    }
  sq[ i ] = 0;
  len = i;  
  
  if ( c != '\n' )
    {
     while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) );
    } 

  return SEQUENCE_READ;
}



int read_seq_len_PHYLIP( FILE *fp, int &nseq, int &len )
{
  int c, i;
  char s[ 101 ];

  while ( ( ( c = fgetc( fp ) ) != EOF ) && !isdigit( c ) );
   
  if ( c == EOF ) return NO_SEQUENCE;

  ungetc( c, fp );

  i = 0;
  while ( ( ( c = fgetc( fp ) ) != EOF ) && isdigit( c ) && ( i < 100 ) ) s[ i++ ] = c;
  s[ i ] = 0;
  nseq = atoi( s );
  
  if ( ( i == 100 ) || ( nseq < 2 ) || ( c == EOF ) ) return NO_SEQUENCE;  

  while ( ( ( c = fgetc( fp ) ) != EOF ) && !isdigit( c ) );
   
  if ( c == EOF ) return NO_SEQUENCE;
  
  ungetc( c, fp );  
  
  i = 0;
  while ( ( ( c = fgetc( fp ) ) != EOF ) && isdigit( c ) && ( i < 100 ) ) s[ i++ ] = c;
  s[ i ] = 0;
  
  if ( i >= 100 ) return SEQUENCE_TOO_LONG;
  
  len = atoi( s );

  if ( len < 0 ) return INVALID_SEQUENCE_LENGTH;

  if ( c != '\n' )
    {
     while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) );
    } 

  return LENGTH_READ;
}



int read_sequences_PHYLIP_SEQUENTIAL( char *fn )
{
  FILE *fp;
  int nseq, len;

  if ( fn[ 0 ] )
    {
     if ( ( fp = fopen( fn, "rt" ) ) == NULL )
       {
        printf( "Error: Unable to open input file ( %s )!\n\n", fn );
        return 0;
       }
    }
  else fp = stdin;

  int l = read_seq_len_PHYLIP( fp, nseq, len );

  if ( l != LENGTH_READ )
    {
     printf( "Error: Failed to read sequence length!\n\n" );
     return 0;
    }

  for ( int i = 0; i < 3; i++ )
    {
     if ( i == 0 ) l = read_next_seq_PHYLIP_SEQUENTIAL( fp, Xid, X, nx, len );
     else if (i == 1) l = read_next_seq_PHYLIP_SEQUENTIAL( fp, Yid, Y, ny, len );
     else l = read_next_seq_PHYLIP_SEQUENTIAL( fp, Zid, Z, nz, len );


     if ( l == SEQUENCE_TOO_LONG )
       {
        printf( "Error: Sequence too long ( > %d )! Please recompile with MAX_SEQ_LENGTH redefined ( i.e., with -DMAX_SEQ_LENGTH=new-length )!\n\n", MAX_SEQ_LENGTH );
        return 0;
       }

     if ( l == NO_SEQUENCE )
       {
        printf( "Error: Failed to read sequence %d!\n\n", i + 1 );
        return 0;
       }
    }

  if ( fn[ 0 ] ) fclose( fp );

  return 1;
}


int read_three_seqs_PHYLIP_INTERLEAVED( FILE *fp, char *xid, char *xsq, int &xlen, char *yid, char *ysq, int &ylen, char *zid, char *zsq, int &zlen, int nseq, int mlen )
{
  int c, i, j, k, len, clen;
  char *sq;
  char *id;

  xlen = ylen = zlen = clen = 0;

  for ( k = 0; k < nseq; k++ )
    {   
     if ( k > 1 )
       {
        while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) );
        continue;
       }         
    
     if ( k == 0 ) sq = xsq, id = xid, len = xlen;
     else if ( k == 1 )sq = ysq, id = yid, len = ylen;
     else sq = zsq, id = zid, len = zlen;

     i = 0;
     while ( ( ( c = fgetc( fp ) ) != EOF ) && ( i < 10 ) ) id[ i++ ] = c;
     id[ i ] = 0;      
     
     i = len;
     j = clen;
     while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) && ( j < mlen ) )
       {
        if ( isspace( c ) || isdigit( c ) || ( c == '.' ) ) continue;
        j++;
        c = toupper( c );     
        if ( strchr( SYM, c ) == NULL ) continue;
        if ( i < MAX_SEQ_LENGTH ) sq[ i++ ] = c;
        else return SEQUENCE_TOO_LONG;
       }
     sq[ i ] = 0;
     len = i;
     
     if ( k == 0 ) xlen = len;
     else ylen = len, clen = j;
    }
         
  while ( clen < mlen )  
    {
     while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c == '\n' ) );
     
     if ( c == EOF ) return NO_SEQUENCE;
     
     ungetc( c, fp );
     
     for ( k = 0; k < nseq; k++ )
       {   
        if ( k > 1 )
          {
           while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) );
           continue;
          }         
       
        if ( k == 0 ) sq = xsq, len = xlen;
        else if ( k == 1) sq = ysq, len = ylen;
        else sq = zsq, len = zlen;

        i = len;
        j = clen;
        while ( ( ( c = fgetc( fp ) ) != EOF ) && ( c != '\n' ) && ( j < mlen ) )
          {
           if ( isspace( c ) || isdigit( c ) || ( c == '.' ) ) continue;
           j++;
           c = toupper( c );     
           if ( strchr( SYM, c ) == NULL ) continue;
           if ( i < MAX_SEQ_LENGTH ) sq[ i++ ] = c;
           else return SEQUENCE_TOO_LONG;
          }
        sq[ i ] = 0;
        len = i;
         
        if ( k == 0 ) xlen = len;
        else ylen = len, clen = j;
       }     
    }

  return SEQUENCE_READ;
}


int read_sequences_PHYLIP_INTERLEAVED( char *fn )
{
  FILE *fp;
  int nseq, len;

  if ( fn[ 0 ] )
    {
     if ( ( fp = fopen( fn, "rt" ) ) == NULL )
       {
        printf( "Error: Unable to open input file ( %s )!\n\n", fn );
        return 0;
       }
    }
  else fp = stdin;

  int l = read_seq_len_PHYLIP( fp, nseq, len );

  if ( l != LENGTH_READ )
    {
     printf( "Error: Failed to read sequence length!\n\n" );
     return 0;
    }
    
  l = read_three_seqs_PHYLIP_INTERLEAVED( fp, Xid, X, nx, Yid, Y, ny, Zid, Z, nz, nseq, len );

  if ( l == SEQUENCE_TOO_LONG )
    {
     printf( "Error: Sequence too long ( > %d )! Please recompile with MAX_SEQ_LENGTH redefined ( i.e., with -DMAX_SEQ_LENGTH=new-length )!\n\n", MAX_SEQ_LENGTH );
     return 0;
    }

  if ( l == NO_SEQUENCE )
    {
     printf( "Error: Failed to read sequences!\n\n" );
     return 0;
    }

  if ( fn[ 0 ] ) fclose( fp );

  return 1;
}



int read_sequences( char *fn, int fmt )
{
  if ( fmt == FASTA ) return read_sequences_FASTA( fn );
  else if ( fmt == PHYLIP_SEQUENTIAL ) return read_sequences_PHYLIP_SEQUENTIAL( fn );
       else if ( fmt == PHYLIP_INTERLEAVED ) return read_sequences_PHYLIP_INTERLEAVED( fn );
  
  return 0;
}


void write_sequences_FASTA( FILE *fp )
{
  char *id;
  vector<char> *sq;
  int i;
  
  for ( int k = 0; k < 4; k++ )
    {
     switch ( k )
       {
        case 0 : sq = &XX, id = Xid;
                 break;
                 
        case 1 : sq = &YY, id = Yid;
                 break;

        case 2 : sq = &ZZ, id = Zid;
                 break;

        case 3 : sq = &Med, id = Medid;
                 break;
       }
     
     fputc( '>', fp ); fputc( ' ', fp );
     i = 0;
     while ( id[ i ] ) fputc( id[ i++ ], fp );
     fputc( '\n', fp );

     i = 0;
          
     while ( i < sq->size() )
       {
        fputc( (*sq)[ i ], fp );
        
        i++;
       
        if ( !( i % 60 ) ) fputc( '\n', fp );
        else if ( !( i % 10 ) ) fputc( ' ', fp );
       }
       
     if ( i % 60 ) fputc( '\n', fp );  
    }
}


void write_sequences_PHYLIP_SEQUENTIAL( FILE *fp )
{
  char *id;
  vector<char> *sq;
  char s[ 100 ];
  int i, j;
  
  i = 0;
  while ( ( strchr( SYM, XX[ i ] ) != NULL ) || ( XX[ i ] == '-' ) ) i++;
  
  sprintf( s, "3 %d\n", i );
  
  i = 0;
  while ( s[ i ] ) fputc( ( int ) s[ i++ ], fp );
  
  for ( int k = 0; k < 4; k++ )
    {
     switch ( k )
       {
        case 0 : sq = &XX, id = Xid;
                 break;
                 
        case 1 : sq = &YY, id = Yid;
                 break;

        case 2 : sq = &ZZ, id = Zid;
                 break;

        case 3 : sq = &Med, id = Medid;
                 break;
       }

     for ( i = j = 0; i < 10; i++ )
       {
        if ( id[ j ] ) fputc( id[ j++ ], fp );
        else fputc( ' ', fp );
       }
        
     fputc( ' ', fp );
        
     i = 0;   
     
     while ( i < sq->size() )
       {
        if ( i && !( i % 60 ) )
          {
           for ( int j = 0; j < 11; j++ )
             fputc( ' ', fp );
          }
          
        fputc( (*sq)[ i ], fp );
        
        i++;
       
        if ( !( i % 60 ) ) fputc( '\n', fp );
        else if ( !( i % 10 ) ) fputc( ' ', fp );
       }
       
     if ( i % 60 ) fputc( '\n', fp );  
    }
}



void write_sequences_PHYLIP_INTERLEAVED( FILE *fp )
{
  char *id;
  vector<char> *sq;
  char s[ 100 ];
  int i, j, l;
  
  i = 0;
  while ( XX[ i ] ) i++;
  
  sprintf( s, "3 %d\n", i );
  
  i = 0;
  while ( s[ i ] ) fputc( ( int ) s[ i++ ], fp );
  
  l = 0;
  for ( int k = 0; k < 4; k++ )
    {
     switch ( k )
       {
        case 0 : sq = &XX, id = Xid;
                 break;
                 
        case 1 : sq = &YY, id = Yid;
                 break;

        case 2 : sq = &ZZ, id = Zid;
                 break;

        case 3 : sq = &Med, id = Medid;
                 break;
       }

     for ( i = j = 0; i < 10; i++ )
       {
        if ( id[ j ] ) fputc( id[ j++ ], fp );
        else fputc( ' ', fp );
       }
       
     fputc( ' ', fp );  
        
     for ( i = 0; i < 60; i++ ) 
       {
        if ( l + i >= sq->size() ) break;
        
        fputc( (*sq)[ l + i ], fp );
        
        if ( ( i + 1 != 60 ) && !( ( i + 1 ) % 10 ) ) fputc( ' ', fp );
       }  
       
     fputc( '\n', fp );  
     
     if ( k == 3 ) l = l + i;
    } 
    
  
  while ( l < XX.size() )  
    {
     fputc( '\n', fp );
     
     for ( int k = 0; k < 4; k++ )
       {
        switch ( k )
          {
           case 0 : sq = &XX, id = Xid;
                    break;
                 
           case 1 : sq = &YY, id = Yid;
                    break;

           case 2 : sq = &ZZ, id = Zid;
                    break;

           case 3 : sq = &Med, id = Medid;
                    break;
          }

    
        for ( j = 0; j < 11; j++ )
           fputc( ' ', fp );
            
        for ( i = 0; i < 60; i++ ) 
          {
           if ( l + i >= sq->size() ) break;
           
           fputc( (*sq)[ l + i ], fp );
            
           if ( ( i + 1 != 60 ) && !( ( i + 1 ) % 10 ) ) fputc( ' ', fp );
          }  
           
        fputc( '\n', fp );  
         
        if ( k == 3 ) l = l + i;
	
       }     
    }
}


int write_sequences( char *fn, int fmt )
{
  FILE *fp;
  
  if ( fn[ 0 ] )
    {
     if ( ( fp = fopen( fn, "wt" ) ) == NULL )
       {
        printf( "Error: Unable to open output file ( %s )!\n\n", fn );
        return 0;
       }
    }
  else fp = stdout;

  if ( fmt == FASTA ) write_sequences_FASTA( fp );
  else if ( fmt == PHYLIP_SEQUENTIAL ) write_sequences_PHYLIP_SEQUENTIAL( fp );
       else if ( fmt == PHYLIP_INTERLEAVED ) write_sequences_PHYLIP_INTERLEAVED( fp );

  if ( fn[ 0 ] ) fclose( fp );
  
  return 1;
}



void print_usage( char *prog )
{
  printf( "Usage: %s [ options ]\n\n", prog );

  printf( "Options:\n" );

  printf( "\t-if name    : input file containing 2 seqeuneces ( default: sequences will be read from stdin )\n" );
  printf( "\t-ifmt f|i|s : input file format: f = FASTA ( default ), i = PHYLIP INTERLEAVED, s = PHYLIP SEQUENTIAL\n\n" );  
  
  printf( "\t-of name    : output file name ( default: output will be written to stdout )\n" );
  printf( "\t-ofmt f|i|s : input file format: f = FASTA ( default ), i = PHYLIP INTERLEAVED, s = PHYLIP SEQUENTIAL\n\n" );    
  
  printf( "\t-o value    : gap open cost ( nonnegative integer, default = %d )\n", DEFAULT_GAP_OPEN_COST );
  printf( "\t-e value    : gap extention cost ( nonnegative integer, default = %d )\n", DEFAULT_GAP_EXTENSION_COST );
  printf( "\t-m value    : mismatch cost ( nonnegative integer, default = %d )\n\n", DEFAULT_MISMATCH_COST );
  
  printf( "\t-b size     : base case size ( positive integer, default = %d )\n", DEFAULT_BASE_SIZE );
  printf( "\t-t value    : maximum number of concurrent threads ( positive integer, default = %d )\n\n", MAX_CONCURRENT_THREADS );    
  
  printf( "\t-c          : compute only the cost of an optimal alignment ( default: compute alignments, too )\n" );
  printf( "\t-d value    : compute alignments if the alignment cost is smaller than the given threshold value\n\n" );
  
  printf( "\t-h          : print this help screen\n\n" );
}



int read_command_line( int argc, char *argv[ ], char *ifname, int &i_fmt, char *ofname, int &o_fmt, int &go_cost, int &ge_cost, int &mm_cost, int &base, int &max_threads, bool &cost_only , int *&est_cost )
{
  go_cost = DEFAULT_GAP_OPEN_COST;
  ge_cost = DEFAULT_GAP_EXTENSION_COST;
  mm_cost = DEFAULT_MISMATCH_COST;
  base = DEFAULT_BASE_SIZE;
  max_threads = MAX_CONCURRENT_THREADS;  
  cost_only = false;
  ifname[ 0 ] = ofname[ 0 ] = 0;
  i_fmt = o_fmt = DEFAULT_FILE_FORMAT;
  est_cost = NULL;

  for ( int i = 1; i < argc; )
    {
     int j = i;
     
     if ( !strcasecmp( argv[ i ], "-o" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing gap open cost ( specify -o gap-open-cost )!\n\n" );
           return 0;
          }

        go_cost = atoi( argv[ i + 1 ] );

        if ( go_cost < 0 )
          {
           printf( "Error: Specify non-negative gap introduction cost!\n\n" );
           return 0;
          }

        i += 2;
        
        if ( i >= argc ) break;
       }
       
       
     if ( !strcasecmp( argv[ i ], "-e" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing gap extention cost ( specify -e gap-extention-cost )!\n\n" );
           return 0;
          }

        ge_cost = atoi( argv[ i + 1 ] );

        if ( ge_cost < 0 )
          {
           printf( "Error: Specify non-negative gap extention cost!\n\n" );
           return 0;
          }

        i += 2;
        
        if ( i >= argc ) break;
       }
     
     
     if ( !strcasecmp( argv[ i ], "-m" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing mismatch cost ( specify -m mismatch-cost )!\n\n" );
           return 0;
          }

        mm_cost = atoi( argv[ i + 1 ] );

        if ( mm_cost < 0 )
          {
           printf( "Error: Specify non-negative mismatch cost!\n\n" );
           return 0;
          }

        i += 2;
        
        if ( i >= argc ) break;
       }
       
       
     if ( !strcasecmp( argv[ i ], "-b" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing base case size ( specify -b size )!\n\n" );
           return 0;
          }
  
        base = atoi( argv[ i + 1 ] );
  
        if ( base <= 0 )
          {
           printf( "Error: Specify positive base size!\n\n" );
           return 0;
          }
 
        i += 2;
        
        if ( i >= argc ) break;
       }


     if ( !strcasecmp( argv[ i ], "-t" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing number of concurrent threads ( specify -t value )!\n\n" );
           return 0;
          }
  
        max_threads = atoi( argv[ i + 1 ] );
  
        if ( max_threads <= 0 )
          {
           printf( "Error: Specify a positive integer for the number of concurrent threads!\n\n" );
           return 0;
          }
 
        i += 2;

        if ( i >= argc ) break;
       }

     if ( !strcasecmp( argv[ i ], "-if" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing input file name ( specify -if file )!\n\n" );
           return 0;
          }

        strcpy( ifname, argv[ i + 1 ] );

        i += 2;
        
        if ( i >= argc ) break;
       }
       
       
     if ( !strcasecmp( argv[ i ], "-of" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing output file name ( specify -of file )!\n\n" );
           return 0;
          }

        strcpy( ofname, argv[ i + 1 ] );

        i += 2;
        
        if ( i >= argc ) break;
       }


     if ( !strcasecmp( argv[ i ], "-ifmt" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing input file format ( specify -ifmt f|i|s )!\n\n" );
           return 0;
          }

        if ( !strcasecmp( argv[ i + 1 ], "f" ) )  i_fmt = FASTA;
        else if ( !strcasecmp( argv[ i + 1 ], "i" ) )  i_fmt = PHYLIP_INTERLEAVED;
             else if ( !strcasecmp( argv[ i + 1 ], "s" ) )  i_fmt = PHYLIP_SEQUENTIAL;
                  else
                    {
                     printf( "Error: Invalid input format ( specify \'f\' or \'i\' or \'s\' )!\n\n" );
                     return 0;
                    }
                    
        i += 2;
        
        if ( i >= argc ) break;
       }
       

     if ( !strcasecmp( argv[ i ], "-ofmt" ) )
       {
        if ( i + 1 >= argc )
          {
           printf( "Error: Missing output file format ( specify -ofmt f|i|s )!\n\n" );
           return 0;
          }

        if ( !strcasecmp( argv[ i + 1 ], "f" ) )  o_fmt = FASTA;
        else if ( !strcasecmp( argv[ i + 1 ], "i" ) )  o_fmt = PHYLIP_INTERLEAVED;
             else if ( !strcasecmp( argv[ i + 1 ], "s" ) )  o_fmt = PHYLIP_SEQUENTIAL;
                  else
                    {
                     printf( "Error: Invalid output format ( specify \'f\' or \'i\' or \'s\' )!\n\n" );
                     return 0;
                    }
                    
        i += 2;
        
        if ( i >= argc ) break;
       }

       
     if ( !strcasecmp( argv[ i ], "-c" ) )
       {
        cost_only = true;
        i++;

        if ( i >= argc ) break;
       }
                 

     if ( !strcasecmp( argv[ i ], "-h" ) || !strcasecmp( argv[ i ], "-help" ) || !strcasecmp( argv[ i ], "--help" ) )
       {
        print_usage( argv[ 0 ] );
        exit( 0 );
       }
       
     if ( !strcasecmp( argv[ i ], "-d" ) )
       {
	 if ( i + 1 >= argc )
	   {
	     printf( "Error: Missing threshold cost ( specify -d value )!\n\n" );
	     return 0;
          }

         if ( !est_cost ) est_cost = new int;
	*est_cost = atoi( argv[ i + 1 ] );

        i += 2;
        
        if ( i >= argc ) break;
       }
     
     if ( i == j )  
       {
        printf( "Error: Unknown option ( %s )!\n\n", argv[ i ] );
        return 0;
       } 
    }
}


int main( int argc, char *argv[ ] )
{
  char ifname[ 200 ], ofname[ 200 ];
  int i_fmt, o_fmt;
  bool cost_only;
  int *est_cost = NULL;
//  double ut, st, tt;
  double et;
  int go_cost, ge_cost, mm_cost;
  DTYPE t;
  int BASE_N, MAX_CONC_THREADS;
  DTYPE ss[ 16 ] = { 0, 1, 1, 1,    /* < A, A >, < A, C >, < A, G >, < A, T > */
                         1, 0, 1, 1,    /* < C, A >, < C, C >, < C, G >, < C, T > */
                         1, 1, 0, 1,    /* < G, A >, < G, C >, < G, G >, < G, T > */
                         1, 1, 1, 0     /* < T, A >, < T, C >, < T, G >, < T, T > */
                       };


  printf( "\nCache-oblivious DP for median of three sequences with affine gap costs ( run with option -h for help ).\n\n" );

  if ( !read_command_line( argc, argv, ifname, i_fmt, ofname, o_fmt, go_cost, ge_cost, mm_cost, BASE_N, MAX_CONC_THREADS, cost_only, est_cost) )
    {
     print_usage( argv[ 0 ] );
     return 1;
    }

  for ( int i = 0; i < 4; i++ )
    for ( int j = 0; j < 4; j++ )
      if ( i != j ) ss[ ( i << 2 ) + j ] = mm_cost;
      else ss[ ( i << 2 ) + j ] = 0;

  printf( "Paremeters:\n\n" );
  printf( "\tgap open cost = %d\n", go_cost );
  printf( "\tgap extention cost = %d\n", ge_cost );
  printf( "\tmismatch cost = %d\n\n", mm_cost );

  printf( "\tbase size = %d\n", BASE_N );
  printf( "\tconcurrent threads = %d\n\n", MAX_CONC_THREADS );    

  if ( ifname[ 0 ] ) printf( "\tinput file = %s\n", ifname );
  else printf( "\tinput file = STANDARD INPUT ( stdin )\n" );

  if ( i_fmt == FASTA ) printf( "\tinput format = FASTA\n\n" );
  else if ( i_fmt == PHYLIP_SEQUENTIAL ) printf( "\tinput format = PHYLIP ( sequential )\n\n" );
       else if ( i_fmt == PHYLIP_INTERLEAVED ) printf( "\tinput format = PHYLIP ( interleaved )\n\n" );

  if ( !read_sequences( ifname, i_fmt ) ) return 1;

  if ( ofname[ 0 ] ) printf( "\toutput file = %s\n", ofname );
  else printf( "\toutput file = STANDARD OUTPUT ( stdout )\n" );

  if ( o_fmt == FASTA ) printf( "\toutput format = FASTA\n\n" );
  else if ( o_fmt == PHYLIP_SEQUENTIAL ) printf( "\toutput format = PHYLIP ( sequential )\n\n" );
       else if ( o_fmt == PHYLIP_INTERLEAVED ) printf( "\toutput format = PHYLIP ( interleaved )\n\n" );

  printf( "Lengths:\n\n" );
  printf( "\tSeq. 1: %d\n", nx );
  printf( "\tSeq. 2: %d\n", ny );
  printf( "\tSeq. 3: %d\n\n", nz );

/*  printf( "Sequences:\n\n" );
  printf( "\tSeq. 1 = %s\n", X );
  printf( "\tSeq. 2 = %s\n\n", Y ); */

  ru.resize( 2 );

  MedianAlign MA(go_cost, ge_cost, ss, BASE_N, MAX_CONC_THREADS );

  MA.setSequences(X, Y, Z, nx, ny, nz);

  if ( cost_only ) printf( "Computing only the cost of an optimal alignment...\n\n" );
  else {
    if ( est_cost ) {
      printf( "Compute an optimal alignment if the alignment cost is smaller than %d...\n\n", *est_cost );
    }
    else printf( "Computing an optimal alignment...\n\n" );
  }

//  getrusage( RUSAGE_SELF, &ru[ 0 ] );
  et = gtod_timer( );    
  if ( cost_only ) t = MA.getAlignmentCost( );
  else {
    if ( est_cost ) {
      t = MA.getBetterAlignments(*est_cost, &XX, &YY, &ZZ, &Med );
    }
    else {
      t = MA.getAlignments( &XX, &YY, &ZZ, &Med );
    }
  }
  et = gtod_timer( ) - et;    
//  getrusage( RUSAGE_SELF, &ru[ 1 ] );

/*  ut =  ru[ 1 ].ru_utime.tv_sec + ( ru[ 1 ].ru_utime.tv_usec * 0.000001 )
      - ( ru[ 0 ].ru_utime.tv_sec + ( ru[ 0 ].ru_utime.tv_usec * 0.000001 ) );
  st =  ru[ 1 ].ru_stime.tv_sec + ( ru[ 1 ].ru_stime.tv_usec * 0.000001 )
      - ( ru[ 0 ].ru_stime.tv_sec + ( ru[ 0 ].ru_stime.tv_usec * 0.000001 ) );
  tt = ut + st; */

  printf( "Wallclock time = %.4lf sec\n\n", et );
//  printf( "Time elapsed: user time = %.4lf sec, system time = %.4lf sec, total time = %.4lf\n\n", ut, st, tt );

  printf( "Optimal cost = %d\n\n", ( int ) t );

  if ( ( cost_only == false ) && ( !est_cost || ( t < *est_cost ) ) )
    {
     write_sequences( ofname, o_fmt );
     if ( !ofname[ 0 ] ) printf( "\n" );
    } 

  return 0;
}
