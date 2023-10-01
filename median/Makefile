C_OPT = -std=c++11 -O3 -pthread -lpthread -DDEBUG=0 -DDEBUG2=0 -DASSERT=0 -DDTYPE="int" -DSPACE_FACTOR=2.2
med: med-3seqs-pt.h med-3seqs-pt.cpp co-median-3seqs-pt.cpp
	g++ $(C_OPT) -c -o med-3seqs-pt.o med-3seqs-pt.cpp
	g++ $(C_OPT) -o med med-3seqs-pt.o co-median-3seqs-pt.cpp
clean:
	rm med-3seqs-pt.o
	rm med
