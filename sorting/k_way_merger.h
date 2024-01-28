/*********************************************************************
*
* k_way_merger
*
* Implementation of a K-way merger factory. This function creates and
* allocates a K-way merger, and returns a pointer to the top BinMerger
* in it.
*
* The space needed is allocated in the array specified by start. The
* first element is placed in start. As a side effect, start is updated
* so that start will always point to the first free location in start.
*
* by: Jesper Holm Olsen and S�ren Skov 2002 University of Copenhagen.
*
*********************************************************************
*
* June 2003: Parameter d introduced
* Frederik R�nn, University of Copenhagen.
*
*********************************************************************/

#ifndef FUNNEL_KMERGER
#define FUNNEL_KMERGER

#include<assert.h>
#include<math.h>
// Computes the size needed for buffers in a k-way merger.
int kSize(int k, int d=3) {
  if (k<=2) {
    return 0; // A BinMerger does need extra buffers.
  }

  int height = ilogb(k);

  int bottomI = height >> 1;
  int topI = height - bottomI;

  int topK = 1 << topI;                 // = 2^(topI)
  int bottomK = 1 << bottomI;        // = 2^(bottomI)

  int sizeOfBuffers = static_cast<int>(pow(k,static_cast<double>((d+1)/2)));

  int totalMidSize = topK * sizeOfBuffers;

  // The size is the middlebuffers + the size of the top-merger + the
  // size of the topK bottomMergers.
  return totalMidSize + kSize(topK, d) + (topK * kSize(bottomK, d));
}


template<class T, class Compare>
BinMerger<T, Compare>* kWayMerger(Buf<T>* out, int k, Buf<T>** in,
				  T*& start, Compare comp, int d) {
  BinMerger<T, Compare>* sources[k];
  for (int i  = 0; i<k; i++) {
    sources[i] = 0;
  }
  return kWayMerger(out, k, in, sources, start, comp, d);
}

template<class T, class Compare>
BinMerger<T, Compare>* kWayMerger(Buf<T>* out, int k, Buf<T>** in,
				  BinMerger<T, Compare>** sources,
				  T*& start, Compare comp, int d) {
  // The base case
  if (k == 2)
    return new BinMerger<T, Compare>(in[0], in[1], out,
				     sources[0], sources[1], comp);

  if (k < 2) {
    // Internal error!
    abort();
  }

  // Compute sizes
  int height = ilogb(k);

  int bottomI = height >> 1;
  int topI = height - bottomI;

  int topK = 1 << topI;                 // = 2^(topI)
  int bottomK = 1 << bottomI;        // = 2^(bottomI)

  int sizeOfBuffers = static_cast<int>(pow(k,static_cast<double>((d+1)/2)));

  // Reserve space for the top merger.
  T* kTopStart = start;
  start += kSize(topK, d);

  // Create middle buffers.
  Buf<T>* midBuffers[topK];

  for (int i = 0; i < topK; i++) {
    midBuffers[i]  = new Buf<T>(start, sizeOfBuffers);
  }

  // Create bottom mergers.
  BinMerger<T, Compare>* bottomMergers[topK];

  for (int i = 0; i < topK; i++) {
    bottomMergers[i] = kWayMerger(midBuffers[i], bottomK,
				  (in + (i*bottomK)), (sources + (i* bottomK)),
				  start, comp, d);
  }

  // Create and return top merger
  BinMerger<T, Compare>* res = kWayMerger(out, topK, midBuffers,
					  bottomMergers, kTopStart, comp, d);
  return res;
}

#endif
