#include "med-3seqs-pt.h"
#include <iostream>

#include <pthread.h>
#include <malloc.h>


const MedianAlign::IndelEntry MedianAlign::indelConfigs[][NSEQUENCES] = {
  {LEFT,LEFT,NONE}, {LEFT,RIGHT,RIGHT}, 
  {LEFT,RIGHT,NONE}, {LEFT,NONE,LEFT}, 
  {LEFT,NONE,RIGHT}, {LEFT,NONE,NONE}, 
  {RIGHT,LEFT,RIGHT}, {RIGHT,LEFT,NONE}, 
  {RIGHT,RIGHT,LEFT}, {RIGHT,RIGHT,RIGHT}, 
  {RIGHT,RIGHT,NONE}, {RIGHT,NONE,LEFT}, 
  {RIGHT,NONE,RIGHT}, {RIGHT,NONE,NONE}, 
  {NONE,LEFT,LEFT}, {NONE,LEFT,RIGHT}, 
  {NONE,LEFT,NONE}, {NONE,RIGHT,LEFT}, 
  {NONE,RIGHT,RIGHT}, {NONE,RIGHT,NONE}, 
  {NONE,NONE,LEFT}, {NONE,NONE,RIGHT}, 
  {NONE,NONE,NONE}, 
};

const MedianAlign::ResidueEntry MedianAlign::residueConfigs[][NSEQUENCES + 1] = {
  {RESIDUE,GAP,GAP,GAP},{GAP,RESIDUE,GAP,GAP},{GAP,GAP,RESIDUE,GAP}, 
  {RESIDUE,GAP,GAP,RESIDUE},{GAP,RESIDUE,GAP,RESIDUE},{GAP,GAP,RESIDUE,RESIDUE},
  {RESIDUE,RESIDUE,GAP,RESIDUE},{RESIDUE,GAP,RESIDUE,RESIDUE},{GAP,RESIDUE,RESIDUE,RESIDUE}, 
  {RESIDUE,RESIDUE,RESIDUE,RESIDUE}
};

const int MedianAlign::XRESIDUE[] = {0, 3};
const int MedianAlign::YRESIDUE[] = {1, 4};
const int MedianAlign::ZRESIDUE[] = {2, 5};

const int MedianAlign::XYRESIDUE[] = {6};
const int MedianAlign::YZRESIDUE[] = {8};
const int MedianAlign::XZRESIDUE[] = {7};

  void MedianAlign::increase_active_threads( void )
    {
     pthread_mutex_lock( &mtx3 );
     active_threads++;
     pthread_mutex_unlock( &mtx3 );
    }

  void MedianAlign::decrease_active_threads( void )
    {
     pthread_mutex_lock( &mtx3 );
     active_threads--;
     pthread_mutex_unlock( &mtx3 );
    }

  void MedianAlign::print_active_threads( void )
    {
     pthread_mutex_lock( &mtx3 );
     printf( "active threads = %d\n", active_threads );
     pthread_mutex_unlock( &mtx3 );
    }


  void MedianAlign::increment_available_threads( void )
    {
     pthread_mutex_lock( &mtx );
     available_threads++;
     pthread_mutex_unlock( &mtx );
    }
  
  
  int MedianAlign::thread_available( void )
    {
     pthread_mutex_lock( &mtx );
     if ( available_threads > 1 )
       {
//        printf( "available threads = %d\n", available_threads );       
        available_threads--;
        pthread_mutex_unlock( &mtx );
        return 1;
       }
     else
       {
        pthread_mutex_unlock( &mtx );
        return 0;
       }
    }
  
  void MedianAlign::wait_for_thread( void )
    {
     while ( 1 )
       {
        pthread_mutex_lock( &mtx );
        if ( available_threads > 0 )
          {
           available_threads--;
           pthread_mutex_unlock( &mtx );
           break;
         }
       else pthread_mutex_unlock( &mtx );
      } 
    }


  void MedianAlign::free_M_chunk( INDEX_TYPE l, INDEX_TYPE h )
    {
     if ( l == h ) return;
     
     pthread_mutex_lock( &mtx3 );

     int i;
     
/*     for ( i = 0; i < free_ptr; i += 2 )
       printf( "< %Ld, %Ld > ", free_pool[ i ], free_pool[ i + 1 ] );
       
     printf( " [ %Ld, %Ld ]\n", l, h );  */
     
     i = free_ptr;
     
     while ( ( h <= free_pool[ i - 2 ] ) )
       {
        free_pool[ i ] = free_pool[ i - 2 ];
        free_pool[ i + 1 ] = free_pool[ i - 1 ];        
        i -= 2;
       }
     
     if ( free_ptr >= FREE_POOL_SIZE ) printf( "free_ptr = %d, FREE_POOL_SIZE = %d\n", free_ptr, FREE_POOL_SIZE );
     
     free_pool[ i ] = l;
     free_pool[ i + 1 ] = h;
     
     free_ptr += 2;     

//     printf( "%Ld, ", topM );
     
     while ( free_pool[ free_ptr - 1 ] == topM )
       {
        topM = free_pool[ free_ptr - 2 ];
        free_ptr -= 2;
       }
       
/*     printf( "%Ld\n", topM );       
     
     for ( i = 0; i < free_ptr; i += 2 )
       printf( "< %Ld, %Ld > ", free_pool[ i ], free_pool[ i + 1 ] );
       
     printf( "\n" );  */

     pthread_mutex_unlock( &mtx3 );
    }


/*
  static methods
*/

void MedianAlign::initGapCostMatrix () {
  int i, j, k;
  DTYPE cost;
  IndelEntry indelCfg[NSEQUENCES];
  
  for (i = 0; i < NINDELCONFIGS; i++){
    for (j = 0; j < NRESIDUECONFIGS; j++){
      cost = 0;
      // consider edge between leaf 0 and internal node
      if (isResidue(residueConfigs[j][NSEQUENCES])) {
	// the internal node is Residue
	for (k = 0; k < NSEQUENCES; k++)
	  if (isResidue(residueConfigs[j][k])) {
	    // both ends are Residue => nochange to cost
	    // the new indel Config is a line
	    indelCfg[k] = NONE;
	  }
	  else{
	    // leaf is gap, internal is Residue
	    // change the indel Config to arrow pointing the internal node (LEFT)
	    cost += (indelConfigs[i][k] == LEFT) ? gapExtendCost : gapIntroCost;
	    indelCfg[k] = LEFT;
	  }
      }// if  
      else{
	// the internal node is Gap
	for (k = 0; k < NSEQUENCES; k++)
	  if (isResidue(residueConfigs[j][k])) {
	    // internal is gap, leaf is Residue
	    // change the indel Config to arrow pointing the leaf (RIGHT)
	    cost += (indelConfigs[i][k] == RIGHT) ? gapExtendCost : gapIntroCost;
	    indelCfg[k] = RIGHT;
	  }
	  else{
	    // both ends are Gap
	    // change nothing
	    indelCfg[k] = indelConfigs[i][k];
	  }
      } //end else
      
	// save to the matrix
      gapCostMatrix[i][j] = cost;
      nextIndelConfigMatrix[i][j] = idxOfIndelCfg(indelCfg);
      if (nextIndelConfigMatrix[i][j] != -1)
	fromIndelConfigMatrix[nextIndelConfigMatrix[i][j]].push_back(FromIndel(i,j, this));
    }
  }
  
#if DEBUG
  printf("%d\n", NINDELCONFIGS);
  for (i = 0; i < NINDELCONFIGS; i++){
    for (j = 0; j < NRESIDUECONFIGS; j++){
      printf("%4d ", nextIndelConfigMatrix[i][j]);
    }
    printf("\n");
  }
  printf("Cost ---------------\n");
  for (i = 0; i < NINDELCONFIGS; i++){
    for (j = 0; j < NRESIDUECONFIGS; j++){
      printf("%4.2f ", gapCostMatrix[i][j]);
    }
    printf("\n");
  }
#endif
}

void MedianAlign::initMatchCostMatrix()
{
  DTYPE val;
  DTYPE min;
  for (int i = 0; i < ALPHABETSIZE; i++)
    for (int j = 0; j < ALPHABETSIZE; j++)
      for (int k = 0; k < ALPHABETSIZE; k++)
	for (int r = 0; r < NRESIDUECONFIGS; r++){
	  min = INF;
	  if (isResidue(residueConfigs[r][NSEQUENCES])){
	    for (int m = 0; m < ALPHABETSIZE; m++) {
	      val = 0;
	      if (residueConfigs[r][0] == RESIDUE)
		val+= costMatrix[m][i];
	      if (residueConfigs[r][1] == RESIDUE)
		val+= costMatrix[m][j];
	      if (residueConfigs[r][2] == RESIDUE)
		val+= costMatrix[m][k];
	      
	      if (val < min) {
		min = matchCostMatrix[i][j][k][r] = val;
		medianResidue[i][j][k][r] = (Alphabet) m;
	      }
	    }
	  }
	  else {
	    matchCostMatrix[i][j][k][r] = 0;
	    medianResidue[i][j][k][r] = X;
	  }
	}
}

void MedianAlign::copyData(DTYPE *s, DTYPE *d)
{
  for (int indel = 0; indel < NINDELCONFIGS; indel++)
    d[indel] = s[indel];
}

void MedianAlign::initInfData(DTYPE *data){
  for (int i = 0 ; i < NINDELCONFIGS; i++)
    data[i] = INF;
}


void MedianAlign::initData(DTYPE *data){
  for (int i = 0 ; i < NINDELCONFIGS; i++)
    data[i] = INF;
  data[NINDELCONFIGS-1] = 0;
}

int MedianAlign::isResidue(ResidueEntry e){
  return e == RESIDUE;
}

void MedianAlign::subtractResidue(int i, int j, int k, int idx[3], int residue){
  idx[0] = i - residueConfigs[residue][0];
  idx[1] = j - residueConfigs[residue][1];
  idx[2] = k - residueConfigs[residue][2];
}

int MedianAlign::idxOfIndelCfg(IndelEntry indelCfg[NSEQUENCES]){
  // sequencial lookup but it is ok
  // because we do it once only and the array quite small
  int i;
  for (i = 0; i < NINDELCONFIGS; i++)
    if( indelConfigs[i][0] == indelCfg[0] &&
	indelConfigs[i][1] == indelCfg[1] &&
	indelConfigs[i][2] == indelCfg[2])
      return i;
  
  //printf("%d %d %d\n", indelCfg[0], indelCfg[1], indelCfg[2]);
  
  return -1;
}

void MedianAlign::print(DTYPE* data){
  for (int i = 0 ; i < NINDELCONFIGS; i++)
    std::cout << data[i] << " " << std::endl;
    //printf("%f ", data[i]);
  printf("\n");
}

int MedianAlign::floatEqual(DTYPE a, DTYPE b)
{      
  if (fabs(a -b) < SMALL)
    return 1;
  return 0;
}

void MedianAlign::printStringsVector(vector<Alphabet> *v)
{
  vector<Alphabet>::reverse_iterator i = v->rbegin();
  vector<Alphabet>::reverse_iterator begin = v->rend();
  while ( i != begin) {
    printf("%c", translate2(*i));
    i++;
  }
}

void MedianAlign::vectorSeqtoString(vector<Alphabet> *v,
				    vector<char> *str)
{
  vector<Alphabet>::reverse_iterator i = v->rbegin();
  vector<Alphabet>::reverse_iterator begin = v->rend();
  while ( i != begin) {
    str->push_back(translate2(*i));
    i++;
  }
}

void MedianAlign:: printStrings(Alphabet *s, int size)
{
  for (int i = 1; i <= size; i++)
    printf("%c", translate2(s[i]));
}

void MedianAlign::printResultStrings()
{
  printf("\nOriginal Strings\n");
  printf("String1 : "); printStrings(string1, SIZEX); printf("\n");
  printf("String2 : "); printStrings(string2, SIZEY); printf("\n");
  printf("String3 : "); printStrings(string3, SIZEZ); printf("\n");
  printf("------------------\n\n");
  printf("String1 : "); printStringsVector(&aligned1); printf("\n");
  printf("String2 : "); printStringsVector(&aligned2); printf("\n");
  printf("String3 : "); printStringsVector(&aligned3); printf("\n");
  printf("Median  : "); printStringsVector(&median); printf("\n");
}

MedianAlign::Alphabet MedianAlign::translate(char c)
{
  switch (c) {
  case 'A':
    return A;
  case 'T':
    return T;
  case 'G':
    return G;
    break;
  case 'C':
    return C;
    break;
  default:
    printf("%c\n", c);
#if ASSERT
    assert(false);
#endif
    return A;
  }
}

char MedianAlign::translate2(Alphabet a)
{
  switch (a) {
  case A:
    return 'A';
  case T:
    return 'T';
  case G:
    return 'G';
  case C:
    return 'C';
  case X:
    return '-';
  default:
#if ASSERT
    assert(false);
#endif
    return '@';
  }
}

int MedianAlign::isPowerOfTwo(int x)
{
  return ((x & (~x+1)) == x);
}

int MedianAlign::logTwo(int x)
{
  int i = -1;
  while (x){
    i++;
    x>>=1;
  }
  return i;
}

/*
  Constructor
 */

/*
MedianAlign::MedianAlign(): MedianAlign(DEFAULT_GAP_OPEN_COST,
					DEFAULT_GAP_EXTENSION_COST,
					DEFAULT_MISMATCH_COST,
					0,
					DEFAULT_BASE_SIZE)
					
{

}
*/


MedianAlign::MedianAlign(DTYPE gi,
			 DTYPE ge,
			 DTYPE ss[],
			 int base, int mx_trd)
{
  for (int i = 0; i < ALPHABETSIZE; i++)
    for (int j = 0; j < ALPHABETSIZE; j++)
      costMatrix[i][j] = ss[(i << 2) + j];

  BASE = base; LOG_BASE = logTwo(BASE);
  
  gapIntroCost = gi;
  gapExtendCost = ge;

  // init
  initGapCostMatrix();
  initMatchCostMatrix();

  BASE_MASK = BASE - 1;
  NOT_BASE_MASK = ~BASE_MASK;

  SIZEONESURFACE = (BASE+1)*(BASE+1);
  SIZEONE = BASE+1;
//  BaseCube = (DTYPE *) malloc((mx_trd + 1)* (BASE + 1)*(BASE + 1)*(BASE + 1) * DATASIZE);
  BaseCube = new DTYPE[ (mx_trd + 1)* (BASE + 1)*(BASE + 1)*(BASE + 1) * NINDELCONFIGS ];
//  BaseCube = ( DTYPE * ) memalign( 64, (mx_trd + 1) *  (BASE + 1) * (BASE + 1) * (BASE + 1) * NINDELCONFIGS * sizeof( DTYPE ) );
  
  MCOST = 0;
  string1 = 0;
  string2 = 0;
  string3 = 0;
  estimate = NULL;
  free_pool = 0;

  pthread_mutex_init( &mtx, NULL );
  max_concurrent_threads = available_threads = mx_trd;  
  
  pthread_mutex_init( &mtx2, NULL );  
//  using_base = ( int * ) malloc( (mx_trd + 1 )* sizeof( int ) ); 
  using_base = new int[ mx_trd + 1 ];
  for ( int i = 0; i <= mx_trd; i++ )
    using_base[ i ] = 0;  
    
  pthread_mutex_init( &mtx3, NULL );  
  active_threads = 1; 
}

// *****************************************************************

MedianAlign::~MedianAlign() {
//  if (MCOST)
//    free(MCOST);
  delete[] MCOST;
//  if (BaseCube)
//    free(BaseCube);
  delete[] BaseCube;
//  if (using_base)
//    free(using_base);
  delete[] using_base;
  delete[] free_pool;
    

  delete[] string1;
  delete[] string2;
  delete[] string3;
}


/*

*/

void MedianAlign::addStrings(int i, int j, int k, int residue)
{
#if ASSERT
#if DEBUG
  printf("Add strings %d %d %d\n", i, j, k);
#endif
  assert (i >= 0 && i <= SIZEX);
  assert (j >= 0 && j <= SIZEY);
  assert (k >= 0 && k <= SIZEZ);
#endif
  
  if (isResidue(residueConfigs[residue][0]))
    aligned1.push_back(string1[i]);
  else
    aligned1.push_back(X);
  
  if (isResidue(residueConfigs[residue][1]))
    aligned2.push_back(string2[j]);
  else
    aligned2.push_back(X);
  
  if (isResidue(residueConfigs[residue][2]))
    aligned3.push_back(string3[k]);
  else
    aligned3.push_back(X);
  
  if (isResidue(residueConfigs[residue][3]))
    median.push_back(medianResidue[string1[i]][string2[j]][string3[k]][residue]);
  else
    median.push_back(X);
#if DEBUG2
  printResultStrings();
#endif
}

// assumption that the surface always starts at the beginnign of BASE
INDEX_TYPE MedianAlign::getLinearIndex(int x, int y, int sizeX, int sizeY)
{
  
#if ASSERT
  assert(x >= 0 && y>=0 && x < sizeX && y < sizeY);
  //printf("HERE %d %d %d %d \n", x, y, sizeX, sizeY);
#endif
  
  int edgeY, edgeX;
  if ( y >= (sizeY & NOT_BASE_MASK))
    edgeY = sizeY & BASE_MASK;
  else
    edgeY= BASE;
  
  if ( x >= (sizeX & NOT_BASE_MASK))
    edgeX = sizeX & BASE_MASK;
  else
    edgeX= BASE;
  
  //printf("%d %d \n", edgeX, edgeY);
  //int idx;
  return ( (y & NOT_BASE_MASK) * sizeX  + ((x & NOT_BASE_MASK) * edgeY ) +
	   ( (y & BASE_MASK) *edgeX ) + (x & BASE_MASK) );
}

// computing G_e^d' int he formula (1) of the paper
DTYPE MedianAlign::computeCost(int i, int j, int k, int residue){
#if ASSERT
  assert(i<= SIZEX && j <= SIZEY && k <= SIZEZ);
  if (i <= 0)
    assert(residueConfigs[residue][0] == GAP);
  if (j <= 0)
    assert(residueConfigs[residue][1] == GAP);
  if (k <= 0)
    assert(residueConfigs[residue][2] == GAP);
#endif
  return matchCostMatrix[string1[i]]
    [string2[j]]
    [string3[k]]
    [residue];
}

void MedianAlign::computeEntrySpecial(int i, int j, int k, int x, int y, int z, DTYPE *BaseCube)
{
  DTYPE *data;
  
  for (int indel = 0; indel < NINDELCONFIGS; indel++)
    {     
      int idx[3];
      DTYPE dv;
      data = &Base(i, j, k, indel);
      *data = INF;
      int size = fromIndelConfigMatrix[indel].size();
      vector<FromIndel>* v = &fromIndelConfigMatrix[indel];
      for (int p = 0 ; p < size; p++) {
	subtractResidue(i, j, k, idx, (*v)[p].residue);
	if (idx[0] < 0 || idx[1] <0 || idx[2] <0)
	  continue;
	dv = Base(idx[0], idx[1], idx[2], (*v)[p].indel)
	      + computeCost(x + i - 1, y + j - 1, z + k - 1, (*v)[p].residue)
	      + (*v)[p].cost;
	*data = min2(*data, dv ) ;	
      }
    }
  
}

void MedianAlign::computeEntry(int i, int j, int k, int x, int y, int z, DTYPE *BaseCube)
{
  DTYPE *data;
  for (int indel = 0; indel < NINDELCONFIGS; indel++)
    {     
      int idx[3];
      DTYPE dv;
      data = &Base(i, j, k, indel);
      *data = INF;
      int size = fromIndelConfigMatrix[indel].size();
      vector<FromIndel>* v = &fromIndelConfigMatrix[indel];
      for (int p = 0 ; p < size; p++) {
	subtractResidue(i, j, k, idx, (*v)[p].residue);
	
	dv = Base(idx[0], idx[1], idx[2], (*v)[p].indel)
	      + computeCost(x + i - 1, y + j - 1, z + k - 1, (*v)[p].residue)
	      + (*v)[p].cost;
	      
	*data = min2( *data, dv ) ;
      }
    }
  
}


void MedianAlign::trace_base(int x, int y, int z,
			     int sizex, int sizey, int sizez, DTYPE *BaseCube)
{

  #if DEBUG
  printf("Bounds [%d %d %d] [%d %d %d]", x, y, z,  fromi, fromj, fromk);
#endif
  // the cube is all filled out. Trace back the strings
  int cidx[3] = {fromi - x + 1, fromj - y + 1, fromk - z + 1};
#if ASSERT
  assert(cidx[0] <= sizex && cidx[1] <= sizey && cidx[2] <= sizez);
#endif
  int indel = fromindel;
  int idx[3];
  int p;


  if (fromi == SIZEX && fromj == SIZEY && fromk == SIZEZ) {
    opt_Cost = INF;
    for (int ind = 0; ind < NINDELCONFIGS; ind++)
      if (Base(cidx[0], cidx[1], cidx[2], ind) < opt_Cost) {
	indel = ind;
	opt_Cost = Base(cidx[0], cidx[1], cidx[2], ind);
      }
    if (estimate && (*estimate < opt_Cost) ) {
      ensureNullEstimate();
      betterThanEstimated = false;
      return;
    }
//    printf("Optimal cost %4.2f \n", opt_Cost);
  }

#if DEBUG
  printf("Trace for [%d %d %d] from [%d %d %d] @ %d cost %f\n", x, y, z,  fromi, fromj, fromk, indel, Base(cidx[0], cidx[1], cidx[2], indel));
#endif
  

  while (cidx[0] != 0 && cidx[1] != 0 && cidx[2] != 0) {
    int size = fromIndelConfigMatrix[indel].size();
    vector<FromIndel>* v = &fromIndelConfigMatrix[indel];
    for (p = 0 ; p < size; p++) {
      subtractResidue(cidx[0], cidx[1], cidx[2], idx, (*v)[p].residue);
      
      if (floatEqual( Base(cidx[0], cidx[1], cidx[2], indel),
		      Base(idx[0], idx[1], idx[2], (*v)[p].indel)
		      + computeCost(cidx[0] + x - 1, cidx[1] +y - 1, cidx[2] + z - 1, (*v)[p].residue)
		      + (*v)[p].cost)) {

	addStrings(cidx[0] + x - 1, cidx[1] + y - 1, cidx[2] + z - 1, (*v)[p].residue);
	cidx[0] = idx[0]; cidx[1] = idx[1];cidx[2] = idx[2];
	indel = (*v)[p].indel;
#if DEBUG
	printf("move to [%d %d %d]\n", cidx[0] + x - 1, cidx[1] + y - 1, cidx[2] + z - 1);
	printf("Cost %f @ indel %d\n", Base(idx[0], idx[1], idx[2], (*v)[p].indel), indel);
	printResultStrings();
#endif
	break;
      }
    }
#if ASSERT
    assert(p != size);
#endif
  }
  // output the top index
  fromi = cidx[0] + x - 1;
  fromj = cidx[1] + y - 1;
  fromk = cidx[2] + z - 1;
  fromindel = indel;
}

void MedianAlign::solve_Front(int x, int y)
{
  INDEX_TYPE tempIdx;
  int cidx[3] = {fromi - x + 1, fromj - y + 1, 0};
  int idx[3];
  int indel = fromindel;
  int p;
  int extra_inc = SIZEX - fromi < BASE ? SIZEX-fromi : BASE-cidx[0];
  
//  DTYPE BaseCube[ (BASE + 1)*(BASE + 1)*(BASE + 1) * DATASIZE ];    

  if (x == 1 && y == 1) {
    copyData(&M(IDX_XYZ, 0),&Base(0, 0, 0, 0));
  }
  else if (y == 1) {
    copyData(&M(IDX_XSTICK + x - 1, 0),&Base(0, 0, 0, 0));
  }
  else {
    copyData(&M(IDX_YSTICK + y - 1, 0),&Base(0, 0, 0, 0));
  }
  
  if (y == 1)
    tempIdx = IDX_XSTICK + x;
  else
    tempIdx = FRONT_IDX + getLinearIndex(x - 1, y-1 - 1, SIZEX, SIZEY);
  for (int i = 1 ; i <= cidx[0]; i++,tempIdx++)
    copyData(&M(tempIdx, 0),&Base(i, 0, 0, 0));

  int step = 0;
  if (x == 1) {
    tempIdx = IDX_YSTICK + y;
    step = 1;
  }
  else {
    tempIdx = FRONT_IDX + getLinearIndex(x-1 - 1, y - 1, SIZEX, SIZEY);
    step = BASE;
  }
    // if the Y-stick is of x=1, z=1, we want to initialize it
  for (int j = 1 ; j <= cidx[1]; j++,tempIdx+=step) 
    copyData(&M(tempIdx, 0),&Base(0, j, 0, 0));

  tempIdx = FRONT_IDX + getLinearIndex(x - 1, y - 1, SIZEX, SIZEY);

  for (int j = 1; j <= cidx[1]; j++) {
    for (int i = 1 ; i <= cidx[0]; i++,tempIdx++)
      copyData(&M(tempIdx, 0),&Base(i, j, 0, 0));
    tempIdx += extra_inc;
  }

  while ( (cidx[0] != 0 || (cidx[0] == 0 && x == 1)) &&
	  (cidx[1] != 0 || (cidx[1] == 0 && y == 1)) ) {
    int size = fromIndelConfigMatrix[indel].size();
    vector<FromIndel>* v = &fromIndelConfigMatrix[indel];
    for (p = 0 ; p < size; p++) 
      if (residueConfigs[(*v)[p].residue][2] == GAP) {
	subtractResidue(cidx[0], cidx[1], cidx[2], idx, (*v)[p].residue);
	
	if (idx[0] < 0 || idx[1] < 0) continue;
	
	if (floatEqual( Base(cidx[0], cidx[1], cidx[2], indel),
			Base(idx[0], idx[1], idx[2],(*v)[p].indel)
			+ computeCost(cidx[0] + x - 1, cidx[1] +y - 1, 0, (*v)[p].residue)
			+ (*v)[p].cost)) {
	  
	  addStrings(cidx[0] + x - 1, cidx[1] + y - 1, 0, (*v)[p].residue);
	  cidx[0] = idx[0]; cidx[1] = idx[1];
	  indel = (*v)[p].indel;
#if DEBUG
	  printf("move to [%d %d %d]\n", cidx[0] + x - 1, cidx[1] + y - 1, 0);
	  printResultStrings();
#endif
	  break;
	}
    }
    
    if (cidx[0] + x - 1 == 0 && cidx[1] + y -1 == 0)
      break;
#if ASSERT
    assert(p!= size);
#endif
  }

  // output the top index
  fromi = cidx[0] + x - 1;
  fromj = cidx[1] + y - 1;
  fromindel = indel;
  
}

void MedianAlign::solve_Top(int y, int z)
{
  INDEX_TYPE tempIdx;
  int cidx[3] = {0, fromj - y + 1, fromk - z + 1};
  int idx[3];
  int indel = fromindel;
  int p;
  int extra_inc = SIZEY - fromj < BASE ? SIZEY-fromj : BASE - cidx[1];
  
//  DTYPE BaseCube[ (BASE + 1)*(BASE + 1)*(BASE + 1) * DATASIZE ];    

//  printf("SolveTop %d %d %d %d %d\n", y, z, cidx[1], cidx[2], extra_inc);
  if (y == 1 && z == 1) {
    copyData(&M(IDX_XYZ, 0),&Base(0, 0, 0, 0));
  }
  else if (z == 1) {
    copyData(&M(IDX_YSTICK + y - 1, 0),&Base(0, 0, 0, 0));
  }
  else {
    copyData(&M(IDX_ZSTICK + z - 1, 0),&Base(0, 0, 0, 0));
  }
  
  if (z == 1)
    tempIdx = IDX_YSTICK + y;
  else
    tempIdx = TOP_IDX + getLinearIndex(y - 1, z-1 - 1 , SIZEY, SIZEZ);
  for (int j = 1 ; j <= cidx[1]; j++,tempIdx++)
    copyData(&M(tempIdx, 0),&Base(0, j, 0, 0));

  int step = 0;
  if (y == 1) {
    tempIdx = IDX_ZSTICK + z;
    step = 1;
  }
  else {
    tempIdx = TOP_IDX + getLinearIndex(y-1 - 1, z - 1, SIZEY, SIZEZ);
    step = BASE;
  }
    // if the Y-stick is of x=1, z=1, we want to initialize it
  for (int k = 1 ; k <= cidx[2]; k++,tempIdx+=step) 
    copyData(&M(tempIdx, 0),&Base(0, 0, k, 0));

  tempIdx = TOP_IDX + getLinearIndex(y - 1, z - 1, SIZEY, SIZEZ);

  for (int k = 1; k <= cidx[2]; k++) {
    for (int  j = 1 ; j <= cidx[1]; j++,tempIdx++)
      copyData(&M(tempIdx, 0),&Base(0, j, k, 0));
    tempIdx += extra_inc;
  }

  while ( (cidx[1] != 0 || (cidx[1] == 0 && y == 1)) &&
	   (cidx[2] != 0 || (cidx[2] == 0 && z == 1)) ) {
    int size = fromIndelConfigMatrix[indel].size();
    vector<FromIndel>* v = &fromIndelConfigMatrix[indel];
//  printf("Top %d %d %f\n", cidx[1], cidx[2], Base(cidx[0], cidx[1], cidx[2], indel));
    for (p = 0 ; p < size; p++) 
      if (residueConfigs[(*v)[p].residue][0] == GAP) {
	subtractResidue(cidx[0], cidx[1], cidx[2], idx, (*v)[p].residue);
	
	if (idx[1] < 0 || idx[2] < 0) continue;

	if (floatEqual( Base(cidx[0], cidx[1], cidx[2], indel),
			Base(idx[0], idx[1], idx[2], (*v)[p].indel)
			+ computeCost(0, cidx[1] + y - 1, cidx[2] + z - 1, (*v)[p].residue)
			+ (*v)[p].cost)) {
	  
	  addStrings(0, cidx[1] + y - 1, cidx[2] + z - 1, (*v)[p].residue);
	  cidx[1] = idx[1]; cidx[2] = idx[2];
	  indel = (*v)[p].indel;
#if DEBUG
	  printf("move to [%d %d %d]\n", 0, cidx[1] + y - 1, cidx[2] + z - 1);
	  printResultStrings();
#endif
	  break;
	}
    }
    
    if (cidx[1] + y - 1 == 0 && cidx[2] + z -1 == 0)
      break;
#if ASSERT
    assert(p!= size);
#endif
  }

  // output the top index
  fromj = cidx[1] + y - 1;
  fromk = cidx[2] + z - 1;
  fromindel = indel;
  
}

void MedianAlign::solve_Left(int x, int z)
{
  INDEX_TYPE tempIdx;
  int cidx[3] = {fromi - x + 1, 0, fromk - z + 1};
  int idx[3];
  int indel = fromindel;
  int p;
  int extra_inc = SIZEX - fromi < BASE ? SIZEX-fromi : BASE-cidx[0];
  
//  DTYPE BaseCube[ (BASE + 1)*(BASE + 1)*(BASE + 1) * DATASIZE ];    

//  printf("SolveLeft %d %d %d %d %d\n", x, z, cidx[0], cidx[2], extra_inc);
  if (x == 1 && z == 1) {
    copyData(&M(IDX_XYZ, 0),&Base(0, 0, 0, 0));
  }
  else if (z == 1) {
    copyData(&M(IDX_XSTICK + x - 1, 0),&Base(0, 0, 0, 0));
  }
  else {
    copyData(&M(IDX_ZSTICK + z - 1, 0),&Base(0, 0, 0, 0));
  }
  
  if (z == 1)
    tempIdx = IDX_XSTICK + x;
  else
    tempIdx = LEFT_IDX + getLinearIndex(x - 1, z-1 - 1, SIZEX, SIZEZ);

  for (int i = 1 ; i <= cidx[0]; i++,tempIdx++)
    copyData(&M(tempIdx, 0),&Base(i, 0, 0, 0));

  int step = 0;
  if (x == 1) {
    tempIdx = IDX_ZSTICK + z;
    step = 1;
  }
  else {
    tempIdx = LEFT_IDX + getLinearIndex(x-1 - 1, z - 1, SIZEX, SIZEZ);
    step = BASE;
  }
    // if the Y-stick is of x=1, z=1, we want to initialize it
  for (int k = 1 ; k <= cidx[2]; k++,tempIdx+=step)
    copyData(&M(tempIdx, 0),&Base(0, 0, k, 0));

  tempIdx = LEFT_IDX + getLinearIndex(x -1 , z -1 ,SIZEX, SIZEZ);

  for (int k = 1; k <= cidx[2]; k++){
    for (int i = 1 ; i <= cidx[0]; i++,tempIdx++)
      copyData(&M(tempIdx, 0),&Base(i, 0, k, 0));
    tempIdx += extra_inc;
  }

  while ( (cidx[0] != 0 || (cidx[0] == 0 && x == 1)) &&
	   (cidx[2] != 0 || (cidx[2] == 0 && z == 1)) ) {
    int size = fromIndelConfigMatrix[indel].size();
    vector<FromIndel>* v = &fromIndelConfigMatrix[indel];

//    printf("Left %d %d\n", cidx[0], cidx[2]);

    for (p = 0 ; p < size; p++) 
      if (residueConfigs[(*v)[p].residue][1] == GAP) {
	subtractResidue(cidx[0], cidx[1], cidx[2], idx, (*v)[p].residue);
	
	if (idx[0] < 0 || idx[2] < 0) continue;
		
	if (floatEqual( Base(cidx[0], cidx[1], cidx[2], indel),
			Base(idx[0], idx[1], idx[2], (*v)[p].indel)
			+ computeCost(cidx[0] + x - 1, 0, cidx[2] + z - 1, (*v)[p].residue)
			+ (*v)[p].cost)) {
	  
	  addStrings(cidx[0] + x - 1, 0, cidx[2] + z - 1, (*v)[p].residue);
	  cidx[0] = idx[0]; cidx[2] = idx[2];
	  indel = (*v)[p].indel;
#if DEBUG
	  printf("move to [%d %d %d]\n", cidx[0] + x - 1, 0, cidx[2] + z - 1);
	  printResultStrings();
#endif
	  break;
	}
    }
    
    if (cidx[0] + x - 1 == 0 && cidx[2] + z -1 == 0)
      break;
#if ASSERT
    assert(p!= size);
#endif
  }

  // output the top index
  fromi = cidx[0] + x - 1;
  fromk = cidx[2] + z - 1;
  fromindel = indel;
  
}

// assume it is the cube
void MedianAlign::solve_base(int x, int y, int z, int lenX, int lenY, int lenZ,
			     INDEX_TYPE FrontIdx, INDEX_TYPE TopIdx, INDEX_TYPE LeftIdx,
			     int FrontSizex, int FrontSizey, int TopSizey, int TopSizez, int LeftSizex, int LeftSizez,
			     int Frontx, int Fronty, int Topy, int Topz, int Leftx, int Leftz,
			     Selector XS, Selector YS, Selector ZS, Selector XYZS, DTYPE *BaseCube)
{

#if ASSERT
  assert(lenX <= BASE && lenY <= BASE && lenZ <= BASE);
#endif
#if DEBUG
  printf("solve_base [%d %d %d] size (%d %d %d): ", x, y,z, lenX, lenY, lenZ);
#endif

  INDEX_TYPE tempIdx;
  
  // copy to the Base cube

  if (x == 1 && y == 1 &&  z == 1) {
    copyData(&M(IDX_XYZ, 0),&Base(0, 0, 0, 0));
  }
  else if (x == 1 && y == 1) {
    copyData(&M(IDX_ZSTICK + z - 1, 0),&Base(0, 0, 0, 0));
  }
  else if (y == 1 && z == 1) {
    copyData(&M(IDX_XSTICK + x - 1, 0),&Base(0, 0, 0, 0));
  }
  else if (x == 1 && z == 1) {
    copyData(&M(IDX_YSTICK + y - 1, 0),&Base(0, 0, 0, 0));
  }
  else {
    if ( XYZS == SF_FRONT)
      copyData(&M(FrontIdx + getLinearIndex(x-1 - Frontx, y-1 - Fronty, FrontSizex, FrontSizey), 0),&Base(0, 0, 0, 0));
    else if (XYZS == SF_TOP)
      copyData(&M(TopIdx + getLinearIndex(y-1 - Topy, z-1 - Topz, TopSizey, TopSizez), 0),&Base(0, 0, 0, 0));
    else if (XYZS == SF_LEFT)
      copyData(&M(LeftIdx + getLinearIndex(x-1 - Leftx, z-1 - Leftz, LeftSizex, LeftSizez), 0),&Base(0, 0, 0, 0));
#if ASSERT
    else
      assert(false);
#endif
  }
  // copy X, Y, Z "stick"

  if (y == 1 && z == 1) {
    tempIdx = IDX_XSTICK + x;
    DTYPE dv1, dv2;
    // if the X-stick is of y=0, z=0, we want to initialize it
    for (int i = 0 ; i < lenX; i++,tempIdx++) {
      initInfData(&Base(i+1, 0, 0, 0));
      for (int indel = 0; indel < NINDELCONFIGS; indel++)
	for (int p = 0 ; p < sizeof(XRESIDUE)/sizeof(int); p++){
	  if (nextIndelConfigMatrix[indel][XRESIDUE[p]] == -1)
	    continue;
	  dv1 = Base(i+1, 0, 0, nextIndelConfigMatrix[indel][XRESIDUE[p]]);
	  dv2 = Base(i, 0, 0, indel)
	    + computeCost(x + i, 0, 0, XRESIDUE[p])
	    + gapCostMatrix[indel][XRESIDUE[p]];
	      
	  Base(i+1, 0, 0, nextIndelConfigMatrix[indel][XRESIDUE[p]]) = min2( dv1, dv2 );
	}
      copyData( &Base(i+1, 0, 0, 0), &M(tempIdx, 0));
    }
  }
  else {
    if ( XS == SF_FRONT)
      tempIdx = FrontIdx + getLinearIndex(x - Frontx, y-1 - Fronty, FrontSizex, FrontSizey);
    else if (XS == SF_LEFT)
      tempIdx = LeftIdx + getLinearIndex(x - Leftx, z-1 - Leftz, LeftSizex, LeftSizez);
#if ASSERT
    else
      assert(false);
#endif
    for (int i = 1 ; i <= lenX; i++,tempIdx++)
      copyData(&M(tempIdx, 0), &Base(i, 0, 0, 0));
  }


  if (x == 1 && z == 1) {
    DTYPE dv1, dv2;  
    tempIdx = IDX_YSTICK + y;
    // if the Y-stick is of x=1, z=1, we want to initialize it
    for (int j = 0 ; j < lenY; j++,tempIdx++) {
      initInfData(&Base(0, j+1, 0, 0));
      for (int indel = 0; indel < NINDELCONFIGS; indel++) {
	for (int p = 0 ; p < sizeof(YRESIDUE)/sizeof(int); p++) {
	  if (nextIndelConfigMatrix[indel][YRESIDUE[p]] == -1)
	    continue;
	  dv1 = Base(0, j+1, 0,  nextIndelConfigMatrix[indel][YRESIDUE[p]]);  
	  dv2 = Base(0, j, 0, indel)
	    + computeCost(0, y + j, 0, YRESIDUE[p])
	    + gapCostMatrix[indel][YRESIDUE[p]];
	    
	  Base(0, j+1, 0, nextIndelConfigMatrix[indel][YRESIDUE[p]]) = min2( dv1, dv2 );
	}
      }
      copyData( &Base(0, j+1, 0, 0), &M(tempIdx, 0));
    }
  }
  else {
    int step = 0;
    
    if (YS == SF_TOP){
      tempIdx = TopIdx + getLinearIndex(y - Topy, z-1 - Topz, TopSizey, TopSizez);
      step = 1;
    }
    else if (YS == SF_FRONT){
      tempIdx = FrontIdx + getLinearIndex(x-1 - Frontx, y - Fronty, FrontSizex, FrontSizey);
      step = BASE;
    }
#if ASSERT
    else
      assert(false);
#endif
    for (int j = 1 ; j <= lenY; j++,tempIdx+=step)
      copyData(&M(tempIdx, 0), &Base(0, j, 0, 0));
  }


  if (x == 1 && y == 1) {
    // if the Y-stick is of x=1, z=1, we want to initialize it
    DTYPE dv1, dv2;
    tempIdx = IDX_ZSTICK + z;
    for (int k = 0 ; k < lenZ; k++,tempIdx++) {
      initInfData(&Base(0, 0, k+1, 0));
      for (int indel = 0; indel < NINDELCONFIGS; indel++) {
	for (int p = 0 ; p < sizeof(ZRESIDUE)/sizeof(int); p++){
	  if (nextIndelConfigMatrix[indel][ZRESIDUE[p]] == -1)
	    continue;
	  dv1 = Base(0, 0, k+1,  nextIndelConfigMatrix[indel][ZRESIDUE[p]]);
	  dv2 = Base(0, 0, k, indel)
	    + computeCost(0, 0, z + k , ZRESIDUE[p])
	    + gapCostMatrix[indel][ZRESIDUE[p]];
	      
	  Base(0, 0, k+1,  nextIndelConfigMatrix[indel][ZRESIDUE[p]]) = min2( dv1, dv2 );
	}
      }
      copyData( &Base(0, 0, k+1, 0), &M(tempIdx, 0));
    }
  }
  else {
    if (ZS == SF_TOP)
      tempIdx = TopIdx + getLinearIndex(y-1 - Topy, z - Topz, TopSizey, TopSizez);
    else if (ZS == SF_LEFT)
      tempIdx = LeftIdx + getLinearIndex(x-1 - Leftx, z - Leftz, LeftSizex, LeftSizez);
#if ASSERT
    else
      assert(false);
#endif
    for (int k = 1 ; k <= lenZ; k++,tempIdx+= BASE)
      copyData(&M(tempIdx, 0), &Base(0, 0, k, 0));
  }

  tempIdx = FrontIdx + getLinearIndex(x - Frontx, y - Fronty, FrontSizex, FrontSizey);
  if ( z == 1) {
    for (int j = 1; j <= lenY; j++)
      for (int i = 1 ; i <= lenX; i++,tempIdx++){
	computeEntrySpecial(i, j, 0, x, y, z, BaseCube);
	copyData(&Base(i, j, 0, 0),  &M(tempIdx, 0));
      }
  }
  else {
    for (int j = 1; j <= lenY; j++)
      for (int i = 1 ; i <= lenX; i++,tempIdx++)
	copyData(&M(tempIdx, 0), &Base(i, j, 0, 0));
  }

  tempIdx = LeftIdx + getLinearIndex(x - Leftx, z - Leftz, LeftSizex, LeftSizez);
  if ( y == 1) {
    for (int k = 1; k <= lenZ; k++)
      for (int i = 1 ; i <= lenX; i++,tempIdx++){
	computeEntrySpecial(i, 0, k, x, y, z, BaseCube);
	copyData(&Base(i, 0, k, 0), &M(tempIdx, 0));
      }
  }
  else {
    for (int k = 1; k <= lenZ; k++)
      for (int i = 1 ; i <= lenX; i++, tempIdx++)
	copyData(&M(tempIdx, 0), &Base(i, 0, k, 0));
  }

  tempIdx = TopIdx + getLinearIndex(y - Topy, z - Topz, TopSizey, TopSizez);
  if ( x == 1) {
    for (int k = 1; k <= lenZ; k++)
      for (int j = 1 ; j <= lenY; j++,tempIdx++){
	computeEntrySpecial(0, j, k, x, y, z, BaseCube);
	copyData(&Base(0, j, k, 0), &M(tempIdx, 0));
      }
  }
  else {
    for (int k = 1; k <= lenZ; k++)
      for (int j = 1 ; j <= lenY; j++, tempIdx++)
	copyData(&M(tempIdx, 0), &Base(0, j, k, 0));
  }

  for (int k = 1; k <= lenZ; k++)
    for (int j = 1 ; j <= lenY; j++)
      for (int i = 1; i <= lenX; i++) {
	computeEntry(i, j, k, x, y, z, BaseCube);
      }

}

void MedianAlign::out_base(int x, int y, int z, int lenX, int lenY, int lenZ,
			   INDEX_TYPE OutFrontIdx, INDEX_TYPE OutTopIdx, INDEX_TYPE OutLeftIdx,
			   int OutFrontSizex, int OutFrontSizey, int OutTopSizey, int OutTopSizez, int OutLeftSizex, int OutLeftSizez,
			   int OutFrontx, int OutFronty, int OutTopy, int OutTopz, int OutLeftx, int OutLeftz, DTYPE *BaseCube)
{
  INDEX_TYPE tempIdx;
  
  // copying values out from the base
  tempIdx = OutFrontIdx + getLinearIndex(x - OutFrontx, y - OutFronty, OutFrontSizex, OutFrontSizey);
 
  //printf("tempIdx %d\n", tempIdx);
  
  for (int j = 1; j <= lenY; j++)
    for (int i = 1 ; i <= lenX; i++,tempIdx++)
      copyData(&Base(i, j, lenZ, 0), &M(tempIdx, 0));

  tempIdx = OutLeftIdx + getLinearIndex(x - OutLeftx, z - OutLeftz, OutLeftSizex, OutLeftSizez);
    
  //printf("tempIdx %d\n", tempIdx);
  for (int k = 1; k <= lenZ; k++)
    for (int i = 1 ; i <= lenX; i++, tempIdx++)
      copyData(&Base(i, lenY, k, 0), &M(tempIdx, 0)); 

  tempIdx = OutTopIdx + getLinearIndex(y - OutTopy, z - OutTopz, OutTopSizey, OutTopSizez);
    
  //printf("tempIdx %d\n", tempIdx);
  for (int k = 1; k <= lenZ; k++)
    for (int j = 1 ; j <= lenY; j++, tempIdx++)
      copyData(&Base(lenX, j, k, 0), &M(tempIdx, 0)); 
}

#define edge(start, len, idx) idx < (start + len)

#define MakeParams(Par, Po, Px,Py,Pz,PlenX,PlenY,PlenZ,PFrontIdx,PTopIdx,PLeftIdx,PFrontSizex,PFrontSizey,PTopSizey,PTopSizez,PLeftSizex,PLeftSizez,PFrontx,PFronty,PTopy,PTopz,PLeftx,PLeftz,PXS,PYS,PZS,PXYZS,POutFrontIdx,POutTopIdx,POutLeftIdx,POutFrontSizex,POutFrontSizey,POutTopSizey,POutTopSizez,POutLeftSizex,POutLeftSizez,POutFrontx,POutFronty,POutTopy,POutTopz,POutLeftx,POutLeftz,Ptrace) \
Par.o = Po; \
Par.x = Px; \
Par.y = Py; \
Par.z = Pz; \
Par.lenX = PlenX; \
Par.lenY = PlenY; \
Par.lenZ = PlenZ; \
Par.FrontIdx = PFrontIdx; \
Par.TopIdx = PTopIdx; \
Par.LeftIdx = PLeftIdx; \
Par.FrontSizex = PFrontSizex; \
Par.FrontSizey = PFrontSizey; \
Par.TopSizey = PTopSizey; \
Par.TopSizez = PTopSizez; \
Par.LeftSizex = PLeftSizex; \
Par.LeftSizez = PLeftSizez; \
Par.Frontx = PFrontx; \
Par.Fronty = PFronty; \
Par.Topy = PTopy; \
Par.Topz = PTopz; \
Par.Leftx = PLeftx; \
Par.Leftz = PLeftz; \
Par.XS = PXS; \
Par.YS = PYS; \
Par.ZS = PZS; \
Par.XYZS = PXYZS; \
Par.OutFrontIdx = POutFrontIdx; \
Par.OutTopIdx = POutTopIdx; \
Par.OutLeftIdx = POutLeftIdx; \
Par.OutFrontSizex = POutFrontSizex; \
Par.OutFrontSizey = POutFrontSizey; \
Par.OutTopSizey = POutTopSizey; \
Par.OutTopSizez = POutTopSizez; \
Par.OutLeftSizex = POutLeftSizex; \
Par.OutLeftSizez = POutLeftSizez; \
Par.OutFrontx = POutFrontx; \
Par.OutFronty = POutFronty; \
Par.OutTopy = POutTopy; \
Par.OutTopz = POutTopz; \
Par.OutLeftx = POutLeftx; \
Par.OutLeftz = POutLeftz; \
Par.trace = Ptrace;
void *MedianAlign::thread_run(void *data)
{
  SOLVE_PARAMS *p = (SOLVE_PARAMS *) data;
//  p->o->increase_active_threads( );                    
//  p->o->print_active_threads( );                          
  p->o->solve_M(p->x,p->y,p->z,p->lenX,p->lenY,p->lenZ,p->FrontIdx,p->TopIdx,p->LeftIdx,p->FrontSizex,p->FrontSizey,p->TopSizey,p->TopSizez,p->LeftSizex,p->LeftSizez,p->Frontx,p->Fronty,p->Topy,p->Topz,p->Leftx,p->Leftz,p->XS,p->YS,p->ZS,p->XYZS,p->OutFrontIdx,p->OutTopIdx,p->OutLeftIdx,p->OutFrontSizex,p->OutFrontSizey,p->OutTopSizey,p->OutTopSizez,p->OutLeftSizex,p->OutLeftSizez,p->OutFrontx,p->OutFronty,p->OutTopy,p->OutTopz,p->OutLeftx,p->OutLeftz,p->trace);
  p->o->increment_available_threads( );                
//  p->o->decrease_active_threads( );                        
}

void MedianAlign::solve_M(int x, int y, int z,  int lenX, int lenY, int lenZ,
			  INDEX_TYPE FrontIdx, INDEX_TYPE TopIdx, INDEX_TYPE LeftIdx,
			  int FrontSizex, int FrontSizey, int TopSizey, int TopSizez, int LeftSizex, int LeftSizez,
			  int Frontx, int Fronty, int Topy, int Topz, int Leftx, int Leftz,
			  Selector XS, Selector YS, Selector ZS, Selector XYZS,
			  INDEX_TYPE OutFrontIdx, INDEX_TYPE OutTopIdx, INDEX_TYPE OutLeftIdx,
			  int OutFrontSizex, int OutFrontSizey, int OutTopSizey, int OutTopSizez, int OutLeftSizex, int OutLeftSizez,
			  int OutFrontx, int OutFronty, int OutTopy, int OutTopz, int OutLeftx, int OutLeftz,
			  TraceOption trace)
{
#if DEBUG
  printf("solve_M size (%d %d %d), idx (%d %d %d) \n", lenX, lenY, lenZ, x, y, z);
  if (trace == TR_YES) {
    printf("Tracing\n");
    if (estimate != null) {
      printf("With estimate %f", *estimate);
    }
    else {
      trace == TR_NO;
      printf("No Tracing since the estimate cost is better\n");
    }
  }
  else {
    printf("No Tracing\n");
  }
#endif
  
#if ASSERT
  assert(lenX >0 && lenY > 0 && lenZ > 0);
#endif
  if (lenX <= BASE && lenY <=BASE && lenZ <= BASE) {
  
    int t;
    pthread_mutex_lock( &mtx2 );
    for ( t = 0; t < max_concurrent_threads; t++ )
      if ( !using_base[ t ] ) break;
    using_base[ t ] = 1;  
    pthread_mutex_unlock( &mtx2 );
  
    if ( t >= max_concurrent_threads ) printf( "error (%d, %d)\n", t, available_threads );
  
    DTYPE *LocalBaseCube;
    
    LocalBaseCube = BaseCube + ( t * (BASE + 1)*(BASE + 1)*(BASE + 1) * NINDELCONFIGS );
    
    solve_base(x, y, z, lenX, lenY, lenZ,
	       FrontIdx, TopIdx, LeftIdx,
	       FrontSizex, FrontSizey, TopSizey, TopSizez, LeftSizex, LeftSizez,
	       Frontx, Fronty, Topy, Topz, Leftx, Leftz,
	       XS, YS, ZS, XYZS, LocalBaseCube);
   
    if (trace == TR_YES) {
      trace_base(x, y, z,
		 lenX, lenY, lenZ, LocalBaseCube);
    }
    else {
      if (x + BASE - 1 == SIZEX && y + BASE - 1 == SIZEY && z + BASE - 1 == SIZEZ) {
	opt_Cost = INF;
	for (int ind = 0; ind < NINDELCONFIGS; ind++)
	  if (LBase(BASE, BASE, BASE, ind) < opt_Cost) {
	    opt_Cost = LBase(BASE, BASE, BASE, ind);
	  }
//	printf("Optimal cost %4.2f \n", opt_Cost);
      }
      
      out_base(x, y, z, lenX, lenY, lenZ,
	       OutFrontIdx, OutTopIdx, OutLeftIdx,
	       OutFrontSizex, OutFrontSizey, OutTopSizey, OutTopSizez, OutLeftSizex, OutLeftSizez,
	       OutFrontx, OutFronty, OutTopy, OutTopz, OutLeftx, OutLeftz, LocalBaseCube);	       	       
    }
    
    pthread_mutex_lock( &mtx2 );
    using_base[ t ] = 0;  
    pthread_mutex_unlock( &mtx2 );
   }
  else {
    
    int newLenX1 = ((lenX+BASE-1)>>1)  & NOT_BASE_MASK;
    int newLenY1 = ((lenY+BASE-1)>>1)  & NOT_BASE_MASK;
    int newLenZ1 = ((lenZ+BASE-1)>>1)  & NOT_BASE_MASK;
    
    if (newLenX1 == 0) newLenX1 = lenX;
    if (newLenY1 == 0) newLenY1 = lenY;
    if (newLenZ1 == 0) newLenZ1 = lenZ;
 
    int newLenX2 = lenX - newLenX1;
    int newLenY2 = lenY - newLenY1;
    int newLenZ2 = lenZ - newLenZ1;

#if DEBUG
    printf(" split [%d %d] [%d %d] [%d %d]\n", newLenX1, newLenX2, newLenY1, newLenY2, newLenZ1, newLenZ2);
#endif

    pthread_mutex_lock( &mtx3 );

    INDEX_TYPE tempIdx = topM;
    INDEX_TYPE idx = tempIdx;
    INDEX_TYPE newOutFrontIdx;
    INDEX_TYPE newOutTopIdx;
    INDEX_TYPE newOutLeftIdx;
    int newOutFrontx, newOutFronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz;
    int newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez;  
   
    if (newLenZ2 > 0) {
      newOutFrontIdx = tempIdx;
      idx += lenX * lenY;
      newOutFrontSizex = lenX; newOutFrontSizey = lenY;
      newOutFrontx = x; newOutFronty = y;
    }
    else {
      newOutFrontIdx = OutFrontIdx;
      newOutFrontSizex = OutFrontSizex; newOutFrontSizey = OutFrontSizey;
      newOutFrontx = OutFrontx; newOutFronty = OutFronty;
    }

    if (newLenX2 > 0) {
      newOutTopIdx = idx;
      idx += lenY * lenZ;
      newOutTopSizey = lenY; newOutTopSizez = lenZ;
      newOutTopy = y; newOutTopz = z;
    }
    else {
      newOutTopIdx = OutTopIdx;
      newOutTopSizey = OutTopSizey; newOutTopSizez = OutTopSizez;
      newOutTopy = OutTopy; newOutTopz = OutTopz;
    }
    
    if (newLenY2 > 0) {
      newOutLeftIdx = idx;
      idx += lenX * lenZ;
      newOutLeftx = x; newOutLeftz = z;
      newOutLeftSizex = lenX; newOutLeftSizez = lenZ;
    }
    else {
      newOutLeftIdx = OutLeftIdx;
      newOutLeftSizex = OutLeftSizex; newOutLeftSizez = OutLeftSizez;
      newOutLeftx = OutLeftx; newOutLeftz = OutLeftz;
    }

    topM = idx;
    
    if ( topM * NINDELCONFIGS >= MSIZE ) printf( "topM = %Ld, MSIZE = %Ld\n", topM * NINDELCONFIGS, MSIZE );
    
    pthread_mutex_unlock( &mtx3 );    

#if PRINT_TOPM
    printf("topM = %d\n", topM);
#endif

/*****************************************
 *  #define for cube generation         
 */

    // cube 1
#define cube1(trace)    solve_M(x, y, z, newLenX1, newLenY1, newLenZ1,          \
				FrontIdx, TopIdx, LeftIdx,                     \
				FrontSizex, FrontSizey, TopSizey, TopSizez, LeftSizex, LeftSizez,   \
				Frontx, Fronty, Topy, Topz, Leftx, Leftz,      \
				XS, YS, ZS, XYZS,                              \
				newOutFrontIdx, newOutTopIdx, newOutLeftIdx,   \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez, \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz, \
				trace);
    
    // cube 2
#define cube2(trace)    solve_M(x+newLenX1, y, z, newLenX2, newLenY1, newLenZ1, \
				FrontIdx, newOutTopIdx, LeftIdx,               \
				FrontSizex, FrontSizey, newOutTopSizey, newOutTopSizez, LeftSizex, LeftSizez,\
				Frontx, Fronty, newOutTopy, newOutTopz, Leftx, Leftz,      \
				XS, SF_FRONT, SF_LEFT, XS,                     \
				newOutFrontIdx, OutTopIdx, newOutLeftIdx,      \
				newOutFrontSizex, newOutFrontSizey, OutTopSizey, OutTopSizez, newOutLeftSizex, newOutLeftSizez,            \
				newOutFrontx, newOutFronty, OutTopy, OutTopz, newOutLeftx, newOutLeftz,                              \
				trace);

    // cube 3
#define cube3(trace)    solve_M(x, y+newLenY1, z, newLenX1, newLenY2, newLenZ1, \
				FrontIdx, TopIdx, newOutLeftIdx,               \
				FrontSizex, FrontSizey, TopSizey, TopSizez, newOutLeftSizex, newOutLeftSizez,\
				Frontx, Fronty, Topy, Topz, newOutLeftx, newOutLeftz,              \
				SF_FRONT, YS, SF_TOP, YS,                      \
				newOutFrontIdx, newOutTopIdx, OutLeftIdx,      \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, OutLeftSizex, OutLeftSizez,    \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, OutLeftx, OutLeftz,                              \
				trace);

    // cube 4
#define cube4(trace)    solve_M(x, y, z+newLenZ1, newLenX1, newLenY1, newLenZ2, \
			       newOutFrontIdx, TopIdx, LeftIdx,                \
			       newOutFrontSizex, newOutFrontSizey, TopSizey, TopSizez, LeftSizex, LeftSizez, \
			       newOutFrontx, newOutFronty, Topy, Topz, Leftx, Leftz,                 \
			       SF_LEFT, SF_TOP, ZS, ZS,                        \
			       OutFrontIdx, newOutTopIdx, newOutLeftIdx,       \
			       OutFrontSizex, OutFrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez, \
			       OutFrontx, OutFronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz, \
			       trace);

    // 2 changes
#define cube5(trace)    solve_M(x+newLenX1, y+newLenY1, z, newLenX2, newLenY2, newLenZ1, \
				FrontIdx, newOutTopIdx, newOutLeftIdx,         \
				FrontSizex, FrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez,\
				Frontx, Fronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz,                     \
				SF_FRONT, SF_FRONT, SF_LEFT, SF_FRONT,         \
				newOutFrontIdx, OutTopIdx, OutLeftIdx,         \
			        newOutFrontSizex, newOutFrontSizey, OutTopSizey, OutTopSizez, OutLeftSizex, OutLeftSizez,\
				newOutFrontx, newOutFronty, OutTopy, OutTopz, OutLeftx, OutLeftz,    \
				trace);

    // fix me

    //cube 6
#define cube6(trace)    solve_M(x, y+newLenY1, z+newLenZ1, newLenX1, newLenY2, newLenZ2, \
				newOutFrontIdx, TopIdx, newOutLeftIdx,         \
				newOutFrontSizex, newOutFrontSizey, TopSizey, TopSizez, newOutLeftSizex, newOutLeftSizez,    \
				newOutFrontx, newOutFronty, Topy, Topz, newOutLeftx, newOutLeftz,                        \
				SF_FRONT, SF_TOP, SF_TOP, SF_TOP,              \
				OutFrontIdx, newOutTopIdx, OutLeftIdx,         \
				OutFrontSizex, OutFrontSizey, newOutTopSizey, newOutTopSizez, OutLeftSizex, OutLeftSizez,\
				OutFrontx, OutFronty, newOutTopy, newOutTopz, OutLeftx, OutLeftz,\
				trace);

#define cube7(trace)    solve_M(x+newLenX1, y, z+newLenZ1, newLenX2, newLenY1, newLenZ2, \
				newOutFrontIdx, newOutTopIdx, LeftIdx,         \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, LeftSizex, LeftSizez,  \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, Leftx, Leftz,                      \
				SF_LEFT, SF_TOP, SF_LEFT, SF_LEFT,             \
				OutFrontIdx, OutTopIdx, newOutLeftIdx,         \
				OutFrontSizex, OutFrontSizey, OutTopSizey, OutTopSizez, newOutLeftSizex, newOutLeftSizez,\
				OutFrontx, OutFronty, OutTopy, OutTopz, newOutLeftx, newOutLeftz, \
				trace);
    
#define cube8(trace)    solve_M(x+newLenX1, y+newLenY1, z+newLenZ1, newLenX2, newLenY2, newLenZ2, \
				newOutFrontIdx, newOutTopIdx, newOutLeftIdx,     \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez,              \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz,              \
				SF_FRONT, SF_TOP, SF_LEFT, SF_FRONT,             \
				OutFrontIdx, OutTopIdx, OutLeftIdx,              \
				OutFrontSizex, OutFrontSizey, OutTopSizey, OutTopSizez, OutLeftSizex, OutLeftSizez,\
				OutFrontx, OutFronty, OutTopy, OutTopz, OutLeftx, OutLeftz, \
				trace);
    // cube 1
#define params1(p,o,trace)    MakeParams(p, o, x, y, z, newLenX1, newLenY1, newLenZ1,          \
				FrontIdx, TopIdx, LeftIdx,                     \
				FrontSizex, FrontSizey, TopSizey, TopSizez, LeftSizex, LeftSizez,   \
				Frontx, Fronty, Topy, Topz, Leftx, Leftz,      \
				XS, YS, ZS, XYZS,                              \
				newOutFrontIdx, newOutTopIdx, newOutLeftIdx,   \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez, \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz, \
				trace);
    
    // params 2
#define params2(p,o,trace)    MakeParams(p, o, x+newLenX1, y, z, newLenX2, newLenY1, newLenZ1, \
				FrontIdx, newOutTopIdx, LeftIdx,               \
				FrontSizex, FrontSizey, newOutTopSizey, newOutTopSizez, LeftSizex, LeftSizez,\
				Frontx, Fronty, newOutTopy, newOutTopz, Leftx, Leftz,      \
				XS, SF_FRONT, SF_LEFT, XS,                     \
				newOutFrontIdx, OutTopIdx, newOutLeftIdx,      \
				newOutFrontSizex, newOutFrontSizey, OutTopSizey, OutTopSizez, newOutLeftSizex, newOutLeftSizez,            \
				newOutFrontx, newOutFronty, OutTopy, OutTopz, newOutLeftx, newOutLeftz,                              \
				trace);
    // params 3
#define params3(p,o,trace)    MakeParams(p, o, x, y+newLenY1, z, newLenX1, newLenY2, newLenZ1, \
				FrontIdx, TopIdx, newOutLeftIdx,               \
				FrontSizex, FrontSizey, TopSizey, TopSizez, newOutLeftSizex, newOutLeftSizez,\
				Frontx, Fronty, Topy, Topz, newOutLeftx, newOutLeftz,              \
				SF_FRONT, YS, SF_TOP, YS,                      \
				newOutFrontIdx, newOutTopIdx, OutLeftIdx,      \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, OutLeftSizex, OutLeftSizez,    \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, OutLeftx, OutLeftz,                              \
				trace);

    // params 4
#define params4(p,o,trace)    MakeParams(p, o, x, y, z+newLenZ1, newLenX1, newLenY1, newLenZ2, \
			       newOutFrontIdx, TopIdx, LeftIdx,                \
			       newOutFrontSizex, newOutFrontSizey, TopSizey, TopSizez, LeftSizex, LeftSizez, \
			       newOutFrontx, newOutFronty, Topy, Topz, Leftx, Leftz,                 \
			       SF_LEFT, SF_TOP, ZS, ZS,                        \
			       OutFrontIdx, newOutTopIdx, newOutLeftIdx,       \
			       OutFrontSizex, OutFrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez, \
			       OutFrontx, OutFronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz, \
			       trace);

    // 2 changes
#define params5(p,o,trace)    MakeParams(p, o, x+newLenX1, y+newLenY1, z, newLenX2, newLenY2, newLenZ1, \
				FrontIdx, newOutTopIdx, newOutLeftIdx,         \
				FrontSizex, FrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez,\
				Frontx, Fronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz,                     \
				SF_FRONT, SF_FRONT, SF_LEFT, SF_FRONT,         \
				newOutFrontIdx, OutTopIdx, OutLeftIdx,         \
			        newOutFrontSizex, newOutFrontSizey, OutTopSizey, OutTopSizez, OutLeftSizex, OutLeftSizez,\
				newOutFrontx, newOutFronty, OutTopy, OutTopz, OutLeftx, OutLeftz,    \
				trace);

    // fix me


    //params 6
#define params6(p,o,trace)    MakeParams(p, o, x, y+newLenY1, z+newLenZ1, newLenX1, newLenY2, newLenZ2, \
				newOutFrontIdx, TopIdx, newOutLeftIdx,         \
				newOutFrontSizex, newOutFrontSizey, TopSizey, TopSizez, newOutLeftSizex, newOutLeftSizez,    \
				newOutFrontx, newOutFronty, Topy, Topz, newOutLeftx, newOutLeftz,                        \
				SF_FRONT, SF_TOP, SF_TOP, SF_TOP,              \
				OutFrontIdx, newOutTopIdx, OutLeftIdx,         \
				OutFrontSizex, OutFrontSizey, newOutTopSizey, newOutTopSizez, OutLeftSizex, OutLeftSizez,\
				OutFrontx, OutFronty, newOutTopy, newOutTopz, OutLeftx, OutLeftz,\
				trace);

#define params7(p,o,trace)    MakeParams(p, o, x+newLenX1, y, z+newLenZ1, newLenX2, newLenY1, newLenZ2, \
				newOutFrontIdx, newOutTopIdx, LeftIdx,         \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, LeftSizex, LeftSizez,  \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, Leftx, Leftz,                      \
				SF_LEFT, SF_TOP, SF_LEFT, SF_LEFT,             \
				OutFrontIdx, OutTopIdx, newOutLeftIdx,         \
				OutFrontSizex, OutFrontSizey, OutTopSizey, OutTopSizez, newOutLeftSizex, newOutLeftSizez,\
				OutFrontx, OutFronty, OutTopy, OutTopz, newOutLeftx, newOutLeftz, \
				trace);
    
#define params8(p,o,trace)    MakeParams(p, o, x+newLenX1, y+newLenY1, z+newLenZ1, newLenX2, newLenY2, newLenZ2, \
				newOutFrontIdx, newOutTopIdx, newOutLeftIdx,     \
				newOutFrontSizex, newOutFrontSizey, newOutTopSizey, newOutTopSizez, newOutLeftSizex, newOutLeftSizez,              \
				newOutFrontx, newOutFronty, newOutTopy, newOutTopz, newOutLeftx, newOutLeftz,              \
				SF_FRONT, SF_TOP, SF_LEFT, SF_FRONT,             \
				OutFrontIdx, OutTopIdx, OutLeftIdx,              \
				OutFrontSizex, OutFrontSizey, OutTopSizey, OutTopSizez, OutLeftSizex, OutLeftSizez,\
				OutFrontx, OutFronty, OutTopy, OutTopz, OutLeftx, OutLeftz, \
				trace);
    /*
     * End of #define
     */

    // always gen cube 1

    // creating threads and synchronized
    // 2,3,4 can be done synchrously
    // 5,6,7 can be done sunchrnously
    cube1(TR_NO);
    
    pthread_t thread_c2, thread_c3, thread_c4;
    pthread_t thread_c5, thread_c6, thread_c7;   
    pthread_attr_t atr;
    size_t st_size;
    SOLVE_PARAMS p2, p3, p4;
    int s[ 3 ];
    
    pthread_attr_init( &atr );
//    pthread_attr_getstacksize( &atr, &st_size );
//    st_size <<= 2;
//    pthread_attr_setstacksize( &atr, st_size );
    
//    printf( "%d\n", st_size );
    
    s[ 0 ] = s[ 1 ] = s[ 2 ] = 0;
        
//    printf("%d %d %d \n", fromi, fromj, fromk);
    if ((trace == TR_NO || !edge(x, newLenX1, fromi)) && newLenX2 > 0){
      params2(p2, this, TR_NO);
//      printf("Creating thread = 2\n");
      if ( s[ 0 ] = thread_available( ) )
         pthread_create( &thread_c2, &atr, (void *(*)(void*)) MedianAlign::thread_run, (void*) &p2);        
      else cube2(TR_NO);	         
    }
//    printf("%d %d %d  from %d %d %d \n", y, newLenY1, newLenY2, fromi, fromj, fromk);
    if ((trace == TR_NO || !edge(y, newLenY1, fromj)) && newLenY2 > 0) {
//      printf("Creating thread = 3\n");
      params3(p3, this, TR_NO);
      if ( s[ 1 ] = thread_available( ) )
         pthread_create( &thread_c3, &atr, (void *(*)(void*)) MedianAlign::thread_run, (void*) &p3);        
      else cube3(TR_NO);	         
    }
//    printf("%d %d %d \n", fromi, fromj, fromk);
    if ((trace == TR_NO || !edge(z, newLenZ1, fromk)) && newLenZ2 > 0){
//      printf("Creating thread = 4\n");
/*      params4(p4, this, TR_NO);
      if ( s[ 2 ] = thread_available( ) )
         pthread_create( &thread_c4, &atr, (void *(*)(void*)) MedianAlign::thread_run, (void*) &p4);
      else */
      cube4(TR_NO);	         
    }

//    increment_available_threads( );
    
    if ( s[ 0 ] ) pthread_join(thread_c2, NULL);
    if ( s[ 1 ] ) pthread_join(thread_c3, NULL);
//    if ( s[ 2 ] ) pthread_join(thread_c4, NULL);

//    wait_for_thread( );                       

    s[ 0 ] = s[ 1 ] = s[ 2 ] = 0;

    if (trace == TR_NO ||
	(!edge(x, newLenX1, fromi) && !edge(y, newLenY1, fromj) && !edge(z, newLenZ1, fromk) ) ){
      if (newLenX2 > 0 && newLenY2 >0){
	params5(p2, this, TR_NO);
	
        if ( s[ 0 ] = thread_available( ) )
           pthread_create( &thread_c5, &atr, (void *(*)(void*)) MedianAlign::thread_run, (void*) &p2);
        else cube5(TR_NO);	         
      }
      if (newLenY2 > 0 && newLenZ2 >0){
	params6(p3, this, TR_NO);

        if ( s[ 1 ] = thread_available( ) )
           pthread_create( &thread_c6, &atr, (void *(*)(void*)) MedianAlign::thread_run, (void*) &p3);
        else cube6(TR_NO);	         
      }
      if (newLenX2 > 0 && newLenZ2 >0){
/*	params7(p4, this, TR_NO);

        if ( s[ 2 ] = thread_available( ) )
           pthread_create( &thread_c7, &atr, (void *(*)(void*)) MedianAlign::thread_run, (void*) &p4);
        else */
        cube7(TR_NO);	         
      }
    }
    
//    increment_available_threads( );
    
    if ( s[ 0 ] ) pthread_join(thread_c5, NULL);
    if ( s[ 1 ] ) pthread_join(thread_c6, NULL);
//    if ( s[ 2 ] ) pthread_join(thread_c7, NULL);

//    wait_for_thread( );                       
        
    if (trace ==TR_NO && newLenX2 > 0 && newLenY2 > 0 && newLenZ2 >0 )
      cube8(TR_NO);

//    printf( "1" ); fflush( stdout );
    
	/*
    if (len == SIZE)
      generatedInitial = 1;  
	*/
    if (trace == TR_YES) {
      // fromi, fromj, fromk tell us which cubic to start next
      while (fromi != (x-1) && fromj != (y-1) && fromk != (z-1)) {
	// if the cost is not better, quit!
	if (!betterThanEstimated) {
	  break;
	}
	//printf("TRace\n");
#if ASSERT
	int handle = 0;
#endif	
	// keep selecting the cube
	if (edge(x, newLenX1,fromi)){
	  // cube 1 ,3 , 4 ,6
	  if (edge(y, newLenY1, fromj)) {
	    // cube 1 or 4
	    if (edge(z, newLenZ1, fromk)){
	      // middle edge of x, y ,z => cube 1
	      cube1(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	    }
	    else {
	      // middle edge of x,y -> cube 4
	      cube4(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	    }
	  }
	  else {
	    // cube 3 or 6
	    if (edge(z, newLenZ1, fromk)) {
	      // cube 3
	      cube3(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	    }
	    else {
	      cube6(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	      //cube 6
	    }
	  }
	}
	else {
	  // cube 2 5 or 7 or 8
	  if (edge(y, newLenY1, fromj)) {
	    // cube 2 or 7
	    if (edge(z, newLenZ1, fromk)){
	      //cube 2
	      cube2(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	    }
	    else {
	      //cube 7
	      cube7(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	    }
	  }
	  else {
	    // cube 5 or 8
	    if (edge(z, newLenZ1, fromk)) {
	      //cube 5
	      cube5(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	    }
	    else {
	      //cube 8
	      cube8(TR_YES);
#if ASSERT
	      handle = 1;
#endif
	    }
	  }
	}
#if ASSERT
	assert(handle);
#endif	
      } //while
    } //if 
    
    
#if ASSERT
    assert(topM == idx);
#endif

    free_M_chunk( tempIdx, idx );

//    printf( "2" ); fflush( stdout );
/*    while ( 1 )
      {
       pthread_mutex_lock( &mtx3 );
       if ( topM == idx ) 
         {
          topM = tempIdx;         
          pthread_mutex_unlock( &mtx3 );                 
          break;
         } 
       else pthread_mutex_unlock( &mtx3 );       
      } */
//    printf( "3" ); fflush( stdout ); 
      
/*    while ( topM != idx ); 
    
    pthread_mutex_lock( &mtx3 );
    topM = tempIdx;         
    pthread_mutex_unlock( &mtx3 );       */
          
  }
}

// *****************************************************************


void MedianAlign::setSequences(char* seqx, char* seqy, char* seqz,
			int sizex, int sizey, int sizez)
{
  SIZEX = sizex;
  SIZEY = sizey;
  SIZEZ = sizez;

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

//  printf("SIZE =  (%d %d %d) BASE = %d MAX_M = %ld VMsize = %ldB\n", SIZEX, SIZEY, SIZEZ, BASE, MAX_M,
//	 (INDEX_TYPE )MAX_M * DATASIZE);

//  if (MCOST)
//    free(MCOST);
  delete[] MCOST;

  //MCOST = (DTYPE *) malloc((MAX_M + BASE*BASE) * DATASIZE);
  MSIZE = (INDEX_TYPE) ( SPACE_FACTOR * (MAX_M + BASE*BASE) * NINDELCONFIGS );
//  MSIZE = (INDEX_TYPE) ( 1.2 * (MAX_M + BASE*BASE) * NINDELCONFIGS );
//  printf( "topM = %Ld, MSIZE = %Ld\n", topM, MSIZE );
  MCOST = new DTYPE[ MSIZE ];
//  MCOST = ( DTYPE * ) memalign( 64, MSIZE * sizeof( DTYPE ) );
  
  for ( int i = 0; i < MSIZE; i++ )
    MCOST[ i ] = 0;
    
  delete[] free_pool;
  FREE_POOL_SIZE = 2 * ( ( SIZEX / BASE ) * ( SIZEY / BASE ) * ( SIZEZ / BASE ) + 1 );
  free_pool = new INDEX_TYPE[ FREE_POOL_SIZE ];  
  free_pool[ 0 ] = free_pool[ 1 ] = -1;
  free_ptr = 2;

  initData(&M(IDX_XYZ, 0));

  // clear out the vectors
  aligned1.clear();
  aligned2.clear();
  aligned3.clear();
  median.clear();  

  // converting the sequences
  delete[] string1;
  delete[] string2;
  delete[] string3;

  string1 = new Alphabet[SIZEX + 1];
  string2 = new Alphabet[SIZEY + 1];
  string3 = new Alphabet[SIZEZ + 1];
  
  //bogus value
  string1[0] = A;
  string2[0] = A;
  string3[0] = A;

  for (int i = 1; i <= SIZEX; i++)
    string1[i] = translate(seqx[i - 1]);
  for (int i = 1; i <= SIZEY; i++)
    string2[i] = translate(seqy[i - 1]);
  for (int i = 1; i <= SIZEZ; i++)
    string3[i] = translate(seqz[i - 1]);
}

void MedianAlign::ensureNullEstimate() {
  if (estimate) {
    delete estimate;
    estimate = NULL;
  }
}

DTYPE MedianAlign::getAlignmentCost()
{  
  ensureNullEstimate();
  solve_M(1, 1, 1, SIZEX, SIZEY, SIZEZ,
	  FRONT_IDX, TOP_IDX, LEFT_IDX,
	  SIZEX, SIZEY, SIZEY, SIZEZ, SIZEX, SIZEZ,
	  1, 1, 1, 1, 1, 1,
	  SF_FRONT, SF_TOP, SF_LEFT, SF_FRONT,
	  OUT_FRONT_IDX, OUT_TOP_IDX, OUT_LEFT_IDX,
	  SIZEX, SIZEY, SIZEY, SIZEZ, SIZEX, SIZEZ,
	  1, 1, 1, 1, 1, 1,
	  TR_NO);

  return opt_Cost;
}

DTYPE MedianAlign::getAlignments(vector<char>* resx, vector<char>* resy , vector<char>* resz, vector<char>* resmedian)
{
  fromi = SIZEX; fromj = SIZEY; fromk = SIZEZ;
  ensureNullEstimate();
  betterThanEstimated = true;
  solve_M(1, 1, 1, SIZEX, SIZEY, SIZEZ,
	  FRONT_IDX, TOP_IDX, LEFT_IDX,
	  SIZEX, SIZEY, SIZEY, SIZEZ, SIZEX, SIZEZ,
	  1, 1, 1, 1, 1, 1,
	  SF_FRONT, SF_TOP, SF_LEFT, SF_FRONT,
	  OUT_FRONT_IDX, OUT_TOP_IDX, OUT_LEFT_IDX,
	  SIZEX, SIZEY, SIZEY, SIZEZ, SIZEX, SIZEZ,
	  1, 1, 1, 1, 1, 1,
	  TR_YES);

#define alignBits(x) x > 0? ((x-1) & NOT_BASE_MASK) + 1 : 1
  // now we need to trace on the surface :(
  if (fromi == 0) {
    while (fromj != 0 || fromk != 0)
      solve_Top(alignBits(fromj), alignBits(fromk));
  }
  else if (fromj == 0) {
    while (fromi != 0 || fromk != 0)
      solve_Left(alignBits(fromi), alignBits(fromk));
  }
  else if (fromk == 0) {
    while (fromi != 0 || fromj != 0)
      solve_Front(alignBits(fromi), alignBits(fromj));
  }

//  printResultStrings();

  // writing the results
  resx->clear();
  resy->clear();
  resz->clear();
  resmedian->clear();  
  
  vectorSeqtoString(&aligned1, resx);
  vectorSeqtoString(&aligned2, resy);
  vectorSeqtoString(&aligned3, resz);
  vectorSeqtoString(&median, resmedian);
  
  return opt_Cost;
}

DTYPE MedianAlign::getBetterAlignments(DTYPE estimatedCost,
				       vector<char>* resx, vector<char>* resy,
				       vector<char>* resz, vector<char>* resmedian)
{
  fromi = SIZEX; fromj = SIZEY; fromk = SIZEZ;
  ensureNullEstimate();
  estimate = new DTYPE;
  *estimate = estimatedCost;
  betterThanEstimated = true;
  solve_M(1, 1, 1, SIZEX, SIZEY, SIZEZ,
	  FRONT_IDX, TOP_IDX, LEFT_IDX,
	  SIZEX, SIZEY, SIZEY, SIZEZ, SIZEX, SIZEZ,
	  1, 1, 1, 1, 1, 1,
	  SF_FRONT, SF_TOP, SF_LEFT, SF_FRONT,
	  OUT_FRONT_IDX, OUT_TOP_IDX, OUT_LEFT_IDX,
	  SIZEX, SIZEY, SIZEY, SIZEZ, SIZEX, SIZEZ,
	  1, 1, 1, 1, 1, 1,
	  TR_YES);
  //if (betterThanEstimated)
  //  printf("Better!\n");
#define alignBits(x) x > 0? ((x-1) & NOT_BASE_MASK) + 1 : 1
  if (betterThanEstimated) {
    // now we need to trace on the surface :(
    if (fromi == 0) {
      while (fromj != 0 || fromk != 0)
	solve_Top(alignBits(fromj), alignBits(fromk));
    }
    else if (fromj == 0) {
      while (fromi != 0 || fromk != 0)
	solve_Left(alignBits(fromi), alignBits(fromk));
    }
    else if (fromk == 0) {
      while (fromi != 0 || fromj != 0)
	solve_Front(alignBits(fromi), alignBits(fromj));
    }
    
    //  printResultStrings();
    
    // writing the results
    resx->clear();
    resy->clear();
    resz->clear();
    resmedian->clear();  
    
    vectorSeqtoString(&aligned1, resx);
    vectorSeqtoString(&aligned2, resy);
    vectorSeqtoString(&aligned3, resz);
    vectorSeqtoString(&median, resmedian);
  }
  return opt_Cost;
}
