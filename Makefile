CC := g++
INCLUDES := -Iinclude
CCFLAGS := -std=c++20 -Wall -pedantic

EXES := Test System Monitor
SOURCES := $(wildcard src/*.cc)
ALL-OBJS := $(patsubst src/%.cc, build/%.o, $(SOURCES))
OBJS := $(filter-out \
  $(addprefix\
    build/,$(addsuffix .o,$(EXES))),\
  $(ALL-OBJS))
LIBS := -lgmpxx -lgmp -lcrypto -lzmq

.PHONY: clean all

all: $(EXES)

clean:
	rm -f build/*.o $(EXES)

Test: $(OBJS) build/Test.o
	$(CC) $(CCFLAGS) -o Test $(OBJS) build/$@.o $(LIBS)

System: $(OBJS) build/System.o
	$(CC) $(CCFLAGS) -o System $(OBJS) build/$@.o $(LIBS)

Monitor: $(OBJS) build/Monitor.o
	$(CC) $(CCFLAGS) -o Monitor $(OBJS) build/$@.o $(LIBS)

build/%.o: src/%.cc
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o $@ $<
