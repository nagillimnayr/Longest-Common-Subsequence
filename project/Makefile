CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++14 -O3 
CXXFLAGS_PARALLEL = -std=c++20 -O3 

SERIAL= lcs_serial
PARALLEL= lcs_parallel_diagonal
DISTRIBUTED= lcs_distributed
HEADERS=cxxopts.hpp timer.h lcs.h
ALL= $(SERIAL) $(PARALLEL) $(DISTRIBUTED)

all : $(ALL)

$(SERIAL): %: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL): %: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS_PARALLEL) -o $@ $<

$(DISTRIBUTED): %: %.cpp  $(HEADERS)
	$(MPICXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
