CC=g++
CFLAGS=-std=c++11 -c -g -Wall
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS)
