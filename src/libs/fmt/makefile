CXX=g++
CXXFLAGS=-std=c++11 -c -g -w -static -I./include
LDFLAGS=-ldl -lpthread -static
SOURCES=$(wildcard src/*.cc)
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=../libs/libfmt.a

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	ar rcs $@ $(OBJECTS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -Rf $(OBJECTS) $(EXECUTABLE)
