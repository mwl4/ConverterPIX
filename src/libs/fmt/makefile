CXX=g++
CXXFLAGS=-c -g -w -static -I./
LDFLAGS=-ldl -lpthread -static
SOURCES=$(wildcard fmt/*.cc)
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=../libs/libfmt.a

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	ar rcs $@ $(OBJECTS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -Rf $(OBJECTS) $(EXECUTABLE)
