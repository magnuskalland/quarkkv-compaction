CC = g++
CFLAGS = -std=c++11 -Wall -Wno-unused-variable -I./fs -I./quark -I./utils -I./core -I./include
LIBS = -lpthread

TARGET = main

SRCS = $(wildcard *.cpp fs/*.cpp quark/*.cpp core/*.cpp utils/*.cpp include/*.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(SRCS:.cpp=.d)

clean:
	rm -f $(OBJS) $(SRCS:.cpp=.d) $(TARGET) ddir/*.sst ddir/MANIFEST-* ddir/CURRENT

.PHONY: all clean
