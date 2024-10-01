CC := g++
CCFLAGS := -std=c++17 -Wall -pedantic

EXES := main System Monitor
SOURCES := $(wildcard src/*.cc)
ALL-OBJS := $(patsubst src/%.cc, build/%.o, $(SOURCES))
OBJS := $(filter-out \
  $(addprefix\
    build/,$(addsuffix .o,$(EXES))),\
  $(ALL-OBJS))
INCLUDES := -Iinclude
LIBS := -lgmp -lcrypto -lzmq

.PHONY: all clean

all: main System Monitor

clean:
	rm -f build/*.o $(EXES)

main: $(OBJS) build/main.o
	$(CC) $(CCFLAGS) -o main $(OBJS) build/$@.o $(LIBS)

System: $(OBJS) build/System.o
	$(CC) $(CCFLAGS) -o System $(OBJS) build/$@.o $(LIBS)

Monitor: $(OBJS) build/Monitor.o
	$(CC) $(CCFLAGS) -o Monitor $(OBJS) build/$@.o $(LIBS)

build/%.o: src/%.cc
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o $@ $<
