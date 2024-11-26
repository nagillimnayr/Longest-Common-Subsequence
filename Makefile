CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++20 -O3 

SERIAL= lcs_serial
PARALLEL= lcs_parallel 
DISTRIBUTED= lcs_distributed
ALL= $(SERIAL) $(PARALLEL) $(DISTRIBUTED)

all : $(ALL)

$(SERIAL): %: %.cpp lcs.h 
	$(CXX) $(CXXFLAGS) -o $@ $<

$(PARALLEL): %: %.cpp lcs.h
	$(CXX) $(CXXFLAGS) -o $@ $<

$(DISTRIBUTED): %: %.cpp lcs.h
	$(MPICXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
