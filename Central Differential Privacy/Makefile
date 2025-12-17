CXX=clang++
CXXFLAGS=-I/usr/local/Cellar -std=c++14 -Wall -O3
EXECUTABLES=frugal_1u_quantile frugal_2u_quantile

all: $(EXECUTABLES)

frugal_1u_quantile: frugal_1u_quantile.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

frugal_2u-quantile: frugal_2u_quantile.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(EXECUTABLES) *.o *~
