/*********************************************************************
*
* funnelSort.h
*
* This .h file defines a cache-oblivious sorting method, based on
* binary mergers.
*
* by: Jesper Holm Olsen and S�ren Skov 2002 University of Copenhagen.
*
*********************************************************************
*
* June 2003: Parameter d introduced
* Frederik R�nn, University of Copenhagen.
*
*********************************************************************/

#include"buf.h"
#include"bin_merge.h"
#include"k_way_merger.h"

#include<algorithm>
#include<math.h>
#include<cstdlib>

namespace FunnelSort {

template <class T, class Compare>
void _sort(T* inFirst, T* inLast, Compare comp, int d, T* space) {
  int size = inLast - inFirst;

  if (size <= 1<<d) {
    // Base case sort with some other sorting algorithm
     std::sort(inFirst, inLast, comp);
    return;
  }

  typedef Buf<T> buf_type;

  // Find the size of the recursive buffers.
  long double Nroot = pow(size, (1.0/d));
  // Round Nroot to a the form 2^x
  int topHeight = static_cast<int>(ilogb(Nroot));
  int topK = 1<<topHeight;

  div_t foo = div(size,topK);
  int bottomSize = foo.rem == 0 ? foo.quot: foo.quot+1;
  int toSort = bottomSize;
  T* fromHere = inFirst;

  buf_type* in[topK];

  // sort the recursive buffers.
  for (int i = 0; i< topK; i++) {
    if (fromHere + bottomSize > inLast) {
      toSort = inLast - fromHere;
    }

    FunnelSort::_sort<T, Compare>(fromHere, fromHere + toSort, comp, d, space);

    // create buffer to represent this sorted input.
    in[i] = new buf_type(fromHere, fromHere + toSort);

    fromHere+= toSort;
  }

  // Create buffer for the output of the funnel
  buf_type out(space, size);

  // Allocate space for K-merger.
  T* start = new T[kSize(topK, d)];
  T* space2 = start;

  // Create the main merger.
  BinMerger<T, Compare>* top;
  top = kWayMerger<T, Compare>(&out, topK, in, start, comp, d);

  top->fill();

  // Copy result back in the range.
  for (int i = 0; i < size; i++) {
    *(inFirst++) = out.extract();
  }

  //Deallocate the space allocated.
  delete top; // Also removes the input buffers.
  delete[] space2;
}

template <class T, class Compare>
void sort(T* inFirst, T* inLast, Compare comp, int d=3) {
  int size = inLast - inFirst;
  // Allocate temporary space used as the output buffer of the funnels .
  T* space = new T[size];

  FunnelSort::_sort(inFirst, inLast, comp, d, space);
  delete[] space;
}


} // end namespace
