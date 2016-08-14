
CXX=g++ -std=c++11
# Includes, cflags
INCLUDES=-I../include -L../lib
CXXOPTS=-Wall -O2
CXXFLAGS=$(CXXOPTS) $(INCLUDES)
# Libs flags
LDLIBS=-lgps -lserial -lpqxx
# Binaries
TARGETS=test testDb

all:$(TARGETS)

clean:
	rm -f $(TARGETS)

default:
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

