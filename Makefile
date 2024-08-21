CC := g++
CCFLAGS := -std=c++17 -Wall -pedantic

SOURCES := $(wildcard src/*.cc)
OBJS := $(patsubst src/%.cc, build/%.o, $(SOURCES))
INCLUDES := -Iinclude
LIBS := -lgmp

.PHONY: all build clean

all: main

build:
	mkdir -p build

clean:
	rm -f build/*.o main

main: $(OBJS)
	$(CC) $(CCFLAGS) -o main $(OBJS) $(LIBS)

build/%.o: src/%.cc build
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o $@ $<
