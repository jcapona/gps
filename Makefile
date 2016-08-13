
CXX=g++ -std=c++11
INCLUDES=-Iinclude -I../serial/include/
CXXOPTS=-Wall -g -O2
CXXFLAGS=$(CXXOPTS) $(INCLUDES)
LDFLAGS=
LDLIBS=
# Where .a file must be created
LIB=lib/libgps.a

default: all
# Object files
OBJS=src/gps.o

all: lib $(LIB)

lib:
	mkdir lib

$(LIB): $(LIB)($(OBJS))

clean:
	rm -f lib/*.a src/*.o

%.d: %.cc
	$(CXX) -MM -MP -MF $@ -MT "$(@:.d=.o) $@" $(INCLUDES) $<

ifneq "$(MAKECMDGOALS)" "clean"
 -include $(OBJS:.o=.d)
endif
