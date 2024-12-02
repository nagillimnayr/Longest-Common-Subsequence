CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++14 -pthread -O3 

SERIAL= lcs_serial
PARALLEL= lcs_parallel
DISTRIBUTED= lcs_distributed
HEADERS=cxxopts.hpp timer.h lcs.h
ALL= $(SERIAL) $(PARALLEL) $(DISTRIBUTED)

all : $(ALL)

$(SERIAL): %: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL): %: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(DISTRIBUTED): %: %.cpp $(HEADERS)
	$(MPICXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
