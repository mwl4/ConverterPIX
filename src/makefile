 #####################################################################
 #           Copyright (C) 2017 mwl4 - All rights reserved           #
 #####################################################################

CXXCOMPILER=g++
CXXSTANDARD=c++14

CXXFLAGS=-c -g -w -O3 -Wall -msse -msse2 -fpermissive -std=$(CXXSTANDARD)

INCLUDES=-I./
INCLUDES+=-I./libs
INCLUDES+=-I./libs/glm
INCLUDES+=-I./libs/fmt

LDFLAGS=-g

LIBS=./libs/libs/libfmt.a
LIBS+=./libs/libs/libcityhash.a
LIBS+=./libs/libs/libzlib.a

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
CXXSOURCE+=$(wildcard pix/*.cpp)

CXXOBJECTS=$(CXXSOURCE:.cpp=.o)

OS=$(shell uname)
ifeq ($(OS),Linux)
	EXECUTABLE=../bin/linux/converter_pix_names
	EXECUTABLE_NO_SYMBOLS=../bin/linux/converter_pix
else
	EXECUTABLE=../bin/macos/converter_pix_names
	EXECUTABLE_NO_SYMBOLS=../bin/macos/converter_pix
endif

NEWLINE="\n"

all: $(EXECUTABLE)
	strip $(EXECUTABLE) -o $(EXECUTABLE_NO_SYMBOLS)
	@echo -e $(NEWLINE) "ConverterPIX has been successfully built!" $(NEWLINE)

$(EXECUTABLE): $(CXXOBJECTS)
	$(CXXCOMPILER) $(CXXOBJECTS) $(LDFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CXXCOMPILER) $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -Rf $(CXXOBJECTS) $(EXECUTABLE) $(EXECUTABLE_NO_SYMBOLS)

# eof #
