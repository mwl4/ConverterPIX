CXX=g++
CXXFLAGS=-c -g -w -static -I./
LDFLAGS=-ldl -lpthread -static
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=../libs/libcityhash.a

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	ar rcs $@ $(OBJECTS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -Rf $(OBJECTS) $(EXECUTABLE)
