#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <float.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#ifndef DEBUG2
#define DEBUG2 1
#endif

#ifndef VERIFY
#define VERIFY 1
#endif

#ifndef ASSERT
#define ASSERT 1
#endif

#ifndef PRINT_TOPM
#define PRINT_TOPM 0
#endif

#define SMALL 0.001f

/*
 * Data type define
 */

#ifndef DTYPE
  #define DTYPE float
#endif

#if DTYPE == int
  #define INF 10000000
#else
  #if #DTYPE == "short int"
    #define INF 20000
  #else
    #define INF INFINITY
  #endif  
#endif

#ifndef ALIGNED
  #define ALIGNED __attribute__( ( aligned( 64 ) ) )
#endif

#ifndef SPACE_FACTOR
  #define SPACE_FACTOR 2.2
#endif

#ifndef INDEX_TYPE
  #define INDEX_TYPE long long
#endif

#define min2( x, y ) ( x ) < ( y ) ? ( x ) : ( y )


using namespace std;

#define NSEQUENCES 3

class MedianAlign {

 private:
  typedef enum alphabet{
    A, T, G, C, X
  } Alphabet;

  // the alphbetsize is 4 {A,T,G,C}
#define ALPHABETSIZE 4


  typedef enum indelEntry {
    LEFT, RIGHT, NONE
  } IndelEntry;

  typedef enum residueEntry {
    GAP, RESIDUE
  } ResidueEntry;

  // All 23 acceptable indelConfigs
  // Convention: LEFT : pointing toward the internal node
  //             RIGHT: pointing away
  // so there are 4 unacceptable sequences:
  // {LEFT,LEFT,LEFT} {LEFT,LEFT,RIGHT}  {RIGHT,LEFT,LEFT} {LEFT,RIGHT,LEFT}
  static const IndelEntry indelConfigs[][NSEQUENCES];

#define NINDELCONFIGS  23
  //(sizeof(indelConfigs) / (NSEQUENCES * sizeof(IndelEntry)))

  //
  // Convention: index 0 is for sequence/leaf 0
  //             index 1 is for sequence/leaf 1
  //             index 2 is for sequence/leaf 2
  //             index 3 is for the internal node
  //
  static const ResidueEntry residueConfigs[][NSEQUENCES + 1];

  static const int XRESIDUE[];
  static const int YRESIDUE[];
  static const int ZRESIDUE[];

  static const int XYRESIDUE[];
  static const int YZRESIDUE[];
  static const int XZRESIDUE[];

#define NRESIDUECONFIGS 10
  //(sizeof(residueConfigs) / ((NSEQUENCES+1) * sizeof(ResidueEntry)))

  DTYPE costMatrix[ALPHABETSIZE][ALPHABETSIZE];/* = {
    {0, DEFAULT_MISMATCH_COST, DEFAULT_MISMATCH_COST, DEFAULT_MISMATCH_COST},
    {DEFAULT_MISMATCH_COST, 0, DEFAULT_MISMATCH_COST, DEFAULT_MISMATCH_COST},
    {DEFAULT_MISMATCH_COST, DEFAULT_MISMATCH_COST, 0, DEFAULT_MISMATCH_COST},
    {DEFAULT_MISMATCH_COST, DEFAULT_MISMATCH_COST, DEFAULT_MISMATCH_COST, 0}
    };*/

  DTYPE gapIntroCost;
  DTYPE gapExtendCost;

  // the G array in formula (1)
  DTYPE gapCostMatrix[NINDELCONFIGS][NRESIDUECONFIGS];

  // the M array in formula (1)
  DTYPE matchCostMatrix[ALPHABETSIZE][ALPHABETSIZE][ALPHABETSIZE][NRESIDUECONFIGS];

  Alphabet medianResidue[ALPHABETSIZE][ALPHABETSIZE][ALPHABETSIZE][NRESIDUECONFIGS];

  int nextIndelConfigMatrix[NINDELCONFIGS][NRESIDUECONFIGS];

  typedef struct fromIndex{
    int indel, residue;
    DTYPE cost;
    fromIndex(int i, int j, MedianAlign *p){indel = i; residue = j;
    cost = p->gapCostMatrix[indel][residue];
    }
  } FromIndel;

  vector<FromIndel> fromIndelConfigMatrix[NINDELCONFIGS];

  /*
   *
   *  Data
   *
   */

  int BASE, LOG_BASE;
  int SIZEX,SIZEY, SIZEZ;
  int BASE_MASK, NOT_BASE_MASK;
  int FREE_POOL_SIZE;

  INDEX_TYPE IDX_XYZ;
  INDEX_TYPE IDX_XSTICK;
  INDEX_TYPE IDX_YSTICK;
  INDEX_TYPE IDX_ZSTICK;
  INDEX_TYPE MAX_M, MSIZE;
  INDEX_TYPE TOP_IDX, FRONT_IDX, LEFT_IDX;
  INDEX_TYPE OUT_TOP_IDX, OUT_FRONT_IDX, OUT_LEFT_IDX;

  INDEX_TYPE topM;

  Alphabet* string1;
  Alphabet* string2;
  Alphabet* string3;

  vector<Alphabet> aligned1, aligned2, aligned3, median;

  DTYPE opt_Cost;

#define DATASIZE sizeof(DTYPE) * NINDELCONFIGS

  int SIZEONESURFACE;
  int SIZEONE;

#define Base(x, y, z, indel) BaseCube[((z) * SIZEONESURFACE + (y) * SIZEONE + (x)) * NINDELCONFIGS + (indel)]
#define LBase(x, y, z, indel) LocalBaseCube[((z) * SIZEONESURFACE + (y) * SIZEONE + (x)) * NINDELCONFIGS + (indel)]

//#define Base(x, y, z, indel) BaseCube[((z) * SIZEONESURFACE + ( ( y ) << LOG_BASE ) + ( y ) + (x)) * NINDELCONFIGS + (indel)]
//#define LBase(x, y, z, indel) LocalBaseCube[((z) * SIZEONESURFACE + ( ( y ) << LOG_BASE ) + ( y ) + (x)) * NINDELCONFIGS + (indel)]

//#define Base(x, y, z, indel) BaseCube[ ( ( ( z ) << ( LOG_BASE + LOG_BASE ) ) + ( ( z ) << ( LOG_BASE + 1 ) ) + ( z ) + ( ( y ) << LOG_BASE ) + ( y ) + ( x ) ) * NINDELCONFIGS + ( indel ) ]
//#define LBase(x, y, z, indel) LocalBaseCube[ ( ( ( z ) << ( LOG_BASE + LOG_BASE ) ) + ( ( z ) << ( LOG_BASE + 1 ) ) + ( z ) + ( ( y ) << LOG_BASE ) + ( y ) + ( x ) ) * NINDELCONFIGS + ( indel ) ]

#define M(x, indel) MCOST[(x) * NINDELCONFIGS + (indel)]

  DTYPE *MCOST;

  // basic cube
  DTYPE *BaseCube;

  typedef enum traceOption{
    TR_YES, TR_NO
  } TraceOption;

  typedef enum selector{
    SF_FRONT, SF_TOP, SF_LEFT
  } Selector;

  typedef struct _solve_params{
    MedianAlign *o;
    int x; int y; int z;  int lenX; int lenY; int lenZ;
    INDEX_TYPE FrontIdx; INDEX_TYPE TopIdx; INDEX_TYPE LeftIdx;
    int FrontSizex; int FrontSizey; int TopSizey; int TopSizez; int LeftSizex; int LeftSizez;
    int Frontx; int Fronty; int Topy; int Topz; int Leftx; int Leftz;
    Selector XS; Selector YS; Selector ZS; Selector XYZS;
    INDEX_TYPE OutFrontIdx; INDEX_TYPE OutTopIdx; INDEX_TYPE OutLeftIdx;
    int OutFrontSizex; int OutFrontSizey; int OutTopSizey; int OutTopSizez; int OutLeftSizex; int OutLeftSizez;
    int OutFrontx; int OutFronty; int OutTopy; int OutTopz; int OutLeftx; int OutLeftz;
    TraceOption trace;
  } SOLVE_PARAMS;

  int fromi, fromj, fromk, fromindel;

  pthread_mutex_t mtx;
  int available_threads;

  pthread_mutex_t mtx2, mtx3;
  int *using_base;
  int max_concurrent_threads, active_threads;

  int free_ptr;
  INDEX_TYPE *free_pool;
  // To save the estimated cost
  DTYPE *estimate;
  bool betterThanEstimated;
 private: //methods
  void increase_active_threads( void );
  void decrease_active_threads( void );
  void print_active_threads( void );

  void increment_available_threads( void );
  int thread_available( void );
  void wait_for_thread( void );

  void free_M_chunk( INDEX_TYPE l, INDEX_TYPE h );

  void copyData(DTYPE *s, DTYPE *d);
  void initInfData(DTYPE *data);
  void initData(DTYPE *data);
  int isResidue(ResidueEntry e);
  void subtractResidue(int i, int j, int k,
					   int idx[3], int residue);
  int idxOfIndelCfg(IndelEntry indelCfg[NSEQUENCES]);

  void print(DTYPE* data);
  void printStringsVector(vector<Alphabet> *v);
  void vectorSeqtoString(vector<Alphabet> *v,
					     vector<char> *str);
  void  printStrings(Alphabet *s, int size);

  int floatEqual(DTYPE a, DTYPE b);
  Alphabet translate(char c);
  char translate2(Alphabet a);

  int isPowerOfTwo(int x);
  int logTwo(int x);

  static void *thread_run(void *data);

  void printResultStrings();

  void initGapCostMatrix ();
  void initMatchCostMatrix();

  void addStrings(int i, int j, int k, int residue);
  INDEX_TYPE getLinearIndex(int x, int y, int sizeX, int sizeY);

  inline DTYPE computeCost(int i, int j, int k, int residue);
  void computeEntrySpecial(int i, int j, int k,
			   int x, int y, int z, DTYPE *BaseCube);
  void computeEntry(int i, int j, int k, int x, int y, int z, DTYPE *BaseCube);

  //computing methods
  void trace_base(int x, int y, int z,
		  int sizex, int sizey, int sizez, DTYPE *BaseCube);
  void solve_Front(int x, int y);
  void solve_Top(int y, int z);
  void solve_Left(int x, int z);

  void solve_base(int x, int y, int z,
		  int lenX, int lenY, int lenZ,
		  INDEX_TYPE FrontIdx, INDEX_TYPE TopIdx, INDEX_TYPE LeftIdx,
		  int FrontSizex, int FrontSizey, int TopSizey,
		  int TopSizez, int LeftSizex, int LeftSizez,
		  int Frontx, int Fronty, int Topy, int Topz,
		  int Leftx, int Leftz,
		  Selector XS, Selector YS, Selector ZS, Selector XYZS, DTYPE *BaseCube);

  void out_base(int x, int y, int z,
		int lenX, int lenY, int lenZ,
		INDEX_TYPE OutFrontIdx, INDEX_TYPE OutTopIdx, INDEX_TYPE OutLeftIdx,
		int OutFrontSizex, int OutFrontSizey, int OutTopSizey, int OutTopSizez,
		int OutLeftSizex, int OutLeftSizez,
		int OutFrontx, int OutFronty, int OutTopy, int OutTopz,
		int OutLeftx, int OutLeftz, DTYPE *BaseCube);

  void solve_M(int x, int y, int z,
	       int lenX, int lenY, int lenZ,
	       INDEX_TYPE FrontIdx, INDEX_TYPE TopIdx, INDEX_TYPE LeftIdx,
	       int FrontSizex, int FrontSizey, int TopSizey, int TopSizez, int LeftSizex, int LeftSizez,
	       int Frontx, int Fronty, int Topy, int Topz, int Leftx, int Leftz,
	       Selector XS, Selector YS, Selector ZS, Selector XYZS,
	       INDEX_TYPE OutFrontIdx, INDEX_TYPE OutTopIdx, INDEX_TYPE OutLeftIdx,
	       int OutFrontSizex, int OutFrontSizey, int OutTopSizey, int OutTopSizez, int OutLeftSizex, int OutLeftSizez,
	       int OutFrontx, int OutFronty, int OutTopy, int OutTopz, int OutLeftx, int OutLeftz,
	       TraceOption trace);
  void ensureNullEstimate();
 public:
  MedianAlign(DTYPE gi, DTYPE ge,
	      DTYPE ss[], int base, int mx_trd);
  ~MedianAlign();

  void setSequences(char* seqx, char* seqy, char* seqz,
		    int sizex, int sizey, int sizez);

  DTYPE getAlignmentCost();
  DTYPE getAlignments(vector<char>* resx, vector<char>* resy,
		      vector<char>* resz, vector<char>* resmedian);
  DTYPE getBetterAlignments(DTYPE estimatedCost,
			    vector<char>* resx, vector<char>* resy,
			    vector<char>* resz, vector<char>* resmedian);

};

#if 0
int main(int argc, char *argv[]){

  char fname[ 200 ];
  /*
    Reading command line options
   */

  printf( "\nCache-oblivious DP for three sequence alignment with affine gap costs ( call with option -h for help ).\n\n" );

  if ( !read_command_line( argc, argv, fname) )
    {
      printf("Wrong options\n");
     return 1;
    }

  printf( "Paremeters:\n\n" );
  printf( "\tgap open cost = %4.2f\n", gapIntroCost);
  printf( "\tgap extention cost = %4.2f\n", gapExtendCost );
  printf( "\tmismatch cost = %4.2f\n\n", costMatrix[0][1]);

  printf( "\tbase size = %d\n\n", BASE);

  if ( fname[ 0 ] ) printf( "\tinput file = %s\n\n", fname );
  else printf( "\tinput file = STANDARD INPUT ( stdin )\n\n" );

  if ( !read_sequences( fname ) ){
    printf("Input from file error\n");
    return 1;
  }
  // init
  initGapCostMatrix();
  initMatchCostMatrix();

  LOG_BASE = logTwo(BASE);

  BASE_MASK = BASE - 1;
  NOT_BASE_MASK = ~BASE_MASK;

  MAX_M = (SIZEX*SIZEY + SIZEY * SIZEZ + SIZEX * SIZEZ )*4 + 1 + SIZEX + SIZEY + SIZEZ;

  IDX_XYZ = 0;
  IDX_XSTICK = IDX_XYZ;
  IDX_YSTICK = IDX_XSTICK + (SIZEX);
  IDX_ZSTICK = IDX_YSTICK + (SIZEY);


  FRONT_IDX = IDX_ZSTICK + (SIZEZ) + 1;
  TOP_IDX = FRONT_IDX + SIZEX * SIZEY;
  LEFT_IDX = TOP_IDX + SIZEY * SIZEZ;

  OUT_FRONT_IDX = LEFT_IDX + (SIZEX * SIZEZ);
  OUT_TOP_IDX = OUT_FRONT_IDX + SIZEX * SIZEY;
  OUT_LEFT_IDX = OUT_TOP_IDX + SIZEY * SIZEZ;

  //  DEFAULT_VALUE_IDX = MAX_M;
  topM = OUT_LEFT_IDX + SIZEX * SIZEZ;

  printf("SIZE =  (%d %d %d) BASE = %d MAX_M = %ld VMsize = %ldB\n", SIZEX, SIZEY, SIZEZ, BASE, MAX_M,
	 (INDEX_TYPE )MAX_M * DATASIZE);

#ifndef USE_STXXL
//  MCOST = (DTYPE *) malloc((MAX_M + BASE*BASE) * DATASIZE);
  MCOST = (DTYPE *) memalign( 64, (MAX_M + BASE*BASE) * DATASIZE );
#else
  MCOST.resize(MAX_M + BASE * BASE);
#endif

  initData(&M(IDX_XYZ, 0));

  SIZEONESURFACE = (BASE+1)*(BASE+1);
  SIZEONE = BASE+1;
//  BaseCube = (DTYPE *) malloc((BASE + 1)*(BASE + 1)*(BASE + 1) * DATASIZE);
  BaseCube = (DTYPE *) memalign( 64, (BASE + 1)*(BASE + 1)*(BASE + 1) * DATASIZE );



    // clean the result strings



#if VERIFY
    //    cubebicDealloc(Base, BASE+1, BASE+1);
    free(BaseCube);
    SIZEONESURFACE = (SIZEX+1)*(SIZEY+1);
    SIZEONE = SIZEX+1;
    BaseCube = (DTYPE *) malloc((SIZEX + 1)*(SIZEY + 1)*(SIZEZ + 1) * DATASIZE);
    initDataBase();
    solve_traditional();
    compareMandBase();
    free(BaseCube);
    //cubicDealloc(Base, SIZE+1, SIZE+1);
    BaseCube = NULL;
#endif
RETURN:
  if (MCOST) free(MCOST);
if(BaseCube) free(BaseCube);
  return 0;
}
#endif


/* FOR DEBUG

#if VERIFY

void initDataBase(){

  initData(&Base(0, 0, 0, 0));
  for (int j = 0 ; j <= SIZEY;  j++)
    for (int i = 0 ; i <= SIZEX;  i++)
      if (i != 0 || j!= 0)
	computeEntrySpecial(i,j, 0, 1 ,1 ,1);

  for (int k = 0 ; k <= SIZEZ; k++)
    for (int j = 0 ; j <= SIZEY; j++)
      if (j != 0 || k!= 0)
	computeEntrySpecial(0 ,j, k, 1 ,1 ,1);

  for (int k = 0 ; k <= SIZEZ;  k++)
    for (int i = 0 ; i <= SIZEX;  i++)
      if (i != 0 || k!= 0)
	computeEntrySpecial(i ,0, k, 1 ,1 ,1);
}

void solve_traditional()
{
  for (int k = 1; k <= SIZEZ; k++)
    for (int j = 1 ; j <= SIZEY; j++)
      for (int i = 1; i <= SIZEX; i++) {
	computeEntry(i, j, k, 1, 1, 1);
      }
}

static int compareEntry(DTYPE* a, DTYPE* b)
{
  for (int i = 0 ; i < NINDELCONFIGS; i++)
    if (fabs(a[i] - b[i]) > SMALL) {
      printf("Mismatch (%d) %f %f ", i, a[i], b[i]);
      return 1;
    }

  return 0;
}

void compareMandBase()
{

  printf("Compare inital values \n");

  for (int i = 1 ; i <= SIZEX; i++)
    if (compareEntry(&Base(i, 0, 0, 0), &M(IDX_XSTICK+i, 0)))
      printf("Mismatch at XSTICK (%d) with Base[%d][%d][%d]\n", i, i, 0, 0);

  for (int j = 1 ; j <= SIZEY; j++)
    if (compareEntry(&Base(0, j, 0, 0), &M(IDX_YSTICK+j, 0)))
      printf("Mismatch at YSTICK (%d) with Base[%d][%d][%d]\n", j, 0, j, 0);

  for (int k = 1 ; k <= SIZEZ; k++)
    if (compareEntry(&Base(0, 0, k , 0), &M(IDX_ZSTICK+k, 0)))
      printf("Mismatch at ZSTICK (%d) with Base[%d][%d][%d]\n", k, 0, 0, k);
  // compare Front
  for (int j = 1; j <= SIZEY; j++)
    for (int i = 1 ; i <= SIZEX; i++){
      int idx = FRONT_IDX + getLinearIndex(i - 1, j - 1, SIZEX, SIZEY);
	if (compareEntry(&Base(i, j, 0, 0), &M(idx, 0)))
	  printf("Mismatch at FRONT (%d,%d) with Base[%d][%d][%d]\n", i-1, j-1, i, j, 0);
    }

  // compare Top
  for (int k = 1; k <= SIZEZ; k++)
    for (int j = 1 ; j <= SIZEY; j++){
      int idx = TOP_IDX + getLinearIndex(j - 1, k - 1, SIZEY, SIZEZ);
	if (compareEntry(&Base(0, j, k, 0), &M(idx, 0)))
	  printf("Mismatch at TOP (%d,%d)\n", j-1, k-1);
    }

  // compare Left
  for (int k = 1; k <= SIZEZ; k++)
    for (int i = 1 ; i <= SIZEX; i++){
      int idx = LEFT_IDX + getLinearIndex(i - 1, k - 1, SIZEX, SIZEZ);
	if (compareEntry(&Base(i, 0, k, 0), &M(idx, 0)))
	  printf("Mismatch at LEFT (%d,%d)\n", i-1, k-1);
    }
  printf("Compare final values \n");
  // compare Front
  for (int j = 1; j <= SIZEY; j++)
    for (int i = 1 ; i <= SIZEX; i++){
      int idx = OUT_FRONT_IDX + getLinearIndex(i - 1, j - 1, SIZEX, SIZEY);
	if (compareEntry(&Base(i, j, SIZEZ, 0), &M(idx, 0)))
	  printf("Mismatch at FRONT (%d,%d)\n", i-1, j-1);
    }

  // compare Top
  for (int k = 1; k <= SIZEZ; k++)
    for (int j = 1 ; j <= SIZEY; j++){
      int idx = OUT_TOP_IDX + getLinearIndex(j - 1, k - 1, SIZEY, SIZEZ);
	if (compareEntry(&Base(SIZEX, j, k, 0), &M(idx, 0)))
	  printf("Mismatch at TOP (%d,%d)\n", j-1, k-1);
    }

  // compare Left
  for (int k = 1; k <= SIZEZ; k++)
    for (int i = 1 ; i <= SIZEX; i++){
      int idx = OUT_LEFT_IDX + getLinearIndex(i - 1, k - 1, SIZEX, SIZEZ);
	if (compareEntry(&Base(i, SIZEY, k, 0), &M(idx, 0)))
	  printf("Mismatch at LEFT (%d,%d)\n", i-1, k-1);
    }


}
#endif
*/
