/*********************************************************************
*
* bin_merge.h
*
* Implementation of a binary merger that merges two streams of sorted
* data. It has two input buffers that it fetches the elements from, if
* one of them runs empty, the corresponding input merger is invoked to
* fill the buffer.
*
* by: Jesper Holm Olsen and S�ren Skov 2002 University of Copenhagen.
*********************************************************************/
#ifndef FUNNEL_BIN
#define FUNNEL_BIN

template<class T, class Compare>
class BinMerger {

public:
  typedef Buf<T> buf_type;
  typedef BinMerger<T, Compare> bin_type;

  buf_type* in1;
  buf_type* in2;
  buf_type* out;
  bin_type* merger1;
  bin_type* merger2;
  Compare comp;

  ~BinMerger() {
    // When a bin merger is killed, it removes all the descendants, and
    // its input buffers. The output buffer is deleted by its parent.
    if (merger1 != 0) delete merger1;
    if (merger2 != 0) delete merger2;
    if (in1 != 0) delete in1;
    if (in2 != 0) delete in2;

  };

  BinMerger(buf_type* in1_, buf_type* in2_, buf_type* out_,
	    bin_type* merger1_, bin_type* merger2_, Compare& comp_)
    : comp(comp_)
  {
    in1 = in1_;
    in2 = in2_;
    out = out_;
    merger1 = merger1_;
    merger2 = merger2_;
  }

  bool exhausted() {
    return ( in1->empty() && (merger1 == 0 || merger1->exhausted()) &&
	     in2->empty() && (merger2 == 0 || merger2->exhausted()));

  }

  void fill() {
    assert(!exhausted());

    bool exhausted1 = false;
    bool exhausted2 = false;

    while ( !(out->full()) ) {
      // Fill empty in buffers.
      if (!exhausted1 && in1->empty()) {
	if (merger1 == 0 || merger1->exhausted()) {
	  exhausted1 = true;
	}
	else {
	  merger1->fill();
	}
      }

      if (!exhausted2 && in2->empty()) {
	if (merger2 == 0 || merger2->exhausted()) {
	  exhausted2 = true;
	}
	else {
	  merger2->fill();
	}
      }

      // Perform one merge step;
      if (exhausted1 && exhausted2)
	return;
      if (exhausted1) {
	out->insert(in2->extract());
	continue;
      }
      if (exhausted2) {
	out->insert(in1->extract());
	continue;
      }

      if (comp(in1->peep(), in2->peep())) {
	out->insert(in1->extract());
      } else {
	out->insert(in2->extract());
      }
    };
  }
};


#endif
