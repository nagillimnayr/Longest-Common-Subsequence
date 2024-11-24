CXX = g++
MPICXX = mpic++
CXXFLAGS = -std=c++14 -O3 

DIAGONAL= lcs_distributed_diagonal
ROW= lcs_distributed_row
COLUMN= lcs_distributed_column
ALL= $(DIAGONAL) $(ROW) $(COLUMN) 

all : $(ALL)

$(DIAGONAL): %: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

$(ROW): %: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

$(COLUMN): %: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
