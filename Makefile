CC = g++
CFLAGS = -std=c++11 -Wno-unused-variable
LIBS = -lpthread 

TARGET = main

SRCS = $(wildcard *.cpp)
HDRS = $(wildcard *.h)

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
