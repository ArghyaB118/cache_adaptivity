/*********************************************************************
*
* buf.h
*
* Implementation of a circular buffer.
*
* by: Jesper Holm Olsen and S�ren Skov 2002 University of Copenhagen.
*********************************************************************
*
* June 2003: Minor customization
* Frederik R�nn, University of Copenhagen.
*
*********************************************************************/


#ifndef FUNNEL_BUF
#define FUNNEL_BUF

#include<functional>
#include<assert.h>
#include<vector>

using namespace std;

template<class T>
class Buf {
  T* start;
  int max_size;


public:
  T* pFirst;
  T* pLast;
  int count;

  Buf(T*& mem, int max_size_) {
    max_size = max_size_;
    pLast = pFirst = start = mem;
    count = 0;
    mem += max_size;
  }

  Buf(T* first, T* last) {
    count = max_size = last - first;
    start = pFirst = first;
    pLast = last - 1;
  };

  ~Buf() {};

  inline void increment(T*& p) const {
    if (p == start + max_size - 1) {
      p = start;
    }
    else {
      ++p;
    }
  }

  inline bool full() { return count == max_size; }

  inline int size() { return count; }

  inline bool empty() { return (count == 0); }

  inline void insert(T elem) {
    assert(!full());
    *pLast = elem;
    increment(pLast);
    ++count;
  }

  inline T extract() {
    assert(!empty());
    T tmp = *pFirst;
    increment(pFirst);
    --count;
    return tmp;
  }

  inline const T peep() {
    assert(!empty());
    return *pFirst;
  }

};

#endif
