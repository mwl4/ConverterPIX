CXXCOMPILER=g++
CXXFLAGS=-c -g -w -O2 -Wall -std=c++14 -msse -msse2 -fpermissive -I./ -I./libs/glm -I./libs/fmt
LDFLAGS=-g ./libs/libs/libfmt.a
CXXSOURCE=$(wildcard *.cpp)
CXXSOURCE+=$(wildcard fs/*.cpp)
CXXSOURCE+=$(wildcard material/*.cpp)
CXXSOURCE+=$(wildcard math/*.cpp)
CXXSOURCE+=$(wildcard model/*.cpp)
CXXSOURCE+=$(wildcard prefab/*.cpp)
CXXSOURCE+=$(wildcard structs/*.cpp)
CXXSOURCE+=$(wildcard texture/*.cpp)
CXXSOURCE+=$(wildcard utils/*.cpp)
CXXSOURCE+=$(wildcard cmd/*.cpp)

CXXOBJECTS=$(CXXSOURCE:.cpp=.o)
EXECUTABLE=../bin/linux/converter_pix_names
EXECUTABLE1=../bin/linux/converter_pix

all: 	$(CXXSOURCE) $(EXECUTABLE) st

st: $(EXECUTABLE)
	strip -s $(EXECUTABLE) -o $(EXECUTABLE1)

$(EXECUTABLE): $(CXXOBJECTS)
	$(CXXCOMPILER) $(CXXOBJECTS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXXCOMPILER) $(CXXFLAGS) $< -o $@

clean:
	rm -Rf $(CXXOBJECTS) $(EXECUTABLE) $(EXECUTABLE1)
