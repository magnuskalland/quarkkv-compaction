CC = g++
# ASAN = -fsanitize=address	
# insert path to your quarklibio
QUARKLIBIO_SRC_DIR = $(abspath ${QUARKSTORE_SRC_DIR}/quarkstore/quarklibio)
QUARKLIBIO_DIR = $(QUARKLIBIO_SRC_DIR)/build

INCLUDE := -I./fs -I./quark -I./utils -I./core -I./iterators -I./ycsb -I./ycsb/core -I$(QUARKLIBIO_SRC_DIR)
LDFLAGS := -L$(QUARKLIBIO_DIR) -lquarklibio
CFLAGS = -std=c++11 -Wall -Wno-unused-variable -O3 -g $(ASAN)

TARGET = main

SRCS = $(wildcard *.cpp fs/*.cpp quark/*.cpp core/*.cpp utils/*.cpp include/*.cpp iterators/*.cpp ycsb/*.cpp ycsb/core/*.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(ASAN)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -MMD -c $< -o $@

-include $(SRCS:.cpp=.d)

clean: del
	rm -f $(OBJS) $(SRCS:.cpp=.d) $(TARGET) *perf*

del:
	rm -f dbdir/*.sst dbdir/MANIFEST-* dbdir/CURRENT

.PHONY: all clean
