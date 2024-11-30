CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++14 -O3 

SERIAL= lcs_serial
PARALLEL= lcs_parallel
DISTRIBUTED= lcs_distributed
DISTRIBUTED_LEN= lcs_distributed_length
DISTRIBUTED_GATHER= lcs_distributed_gather
HEADERS=cxxopts.hpp timer.h lcs.h
ALL= $(SERIAL) $(PARALLEL) $(DISTRIBUTED)

all : $(ALL)

$(SERIAL): %: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL): %: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(DISTRIBUTED): %: distributed/%.cpp $(HEADERS) distributed/lcs_distributed.h
	$(MPICXX) $(CXXFLAGS) -o $@ $<

$(DISTRIBUTED_LEN): %: distributed/%.cpp $(HEADERS) distributed/lcs_distributed.h
	$(MPICXX) $(CXXFLAGS) -o $@ $<

$(DISTRIBUTED_GATHER): %: distributed/%.cpp $(HEADERS) distributed/lcs_distributed.h
	$(MPICXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
