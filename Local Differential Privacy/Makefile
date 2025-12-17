CXX=g++
CXXFLAGS=-std=c++14 -O3
EXECUTABLES= ezq-sw ldpq frugal2u-sw frugal1u-rr

all: $(EXECUTABLES)

ezq-sw: ezq-sw.cpp
	$(CXX) $(CXXFLAGS) -o $@ ezq-sw.cpp QuickSelect.cpp

frugal1u-rr: frugal1u-rr.cpp
	$(CXX) $(CXXFLAGS) -o $@ frugal1u-rr.cpp QuickSelect.cpp

frugal2u-sw: frugal2u-sw.cpp
	$(CXX) $(CXXFLAGS) -o $@ frugal2u-sw.cpp QuickSelect.cpp

ldpq: ldpq.cpp
	$(CXX) $(CXXFLAGS) -o $@ ldpq.cpp QuickSelect.cpp

clean:
	rm -f $(EXECUTABLES) *.o *~
