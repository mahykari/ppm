CC := g++
INCLUDES := -Iinclude
CCFLAGS := -std=c++17 -Wall -pedantic

EXES := Test System Monitor
SOURCES := $(wildcard src/*.cc)
ALL-OBJS := $(patsubst src/%.cc, build/%.o, $(SOURCES))
OBJS := $(filter-out \
  $(addprefix\
    build/,$(addsuffix .o,$(EXES))),\
  $(ALL-OBJS))
LIBS := -lgmpxx -lgmp -lcrypto -lzmq

.PHONY: ignore all clean

all: Test System Monitor

ignore:
	@echo "Adding EXES to .gitignore ..."
	sed -i '/# binary output(s)/{n;s/.*/$(EXES)/}' .gitignore

clean:
	rm -f build/*.o bin/*

Test: $(OBJS) build/Test.o
	$(CC) $(CCFLAGS) -o bin/Test $(OBJS) build/$@.o $(LIBS)

System: $(OBJS) build/System.o
	$(CC) $(CCFLAGS) -o bin/System $(OBJS) build/$@.o $(LIBS)

Monitor: $(OBJS) build/Monitor.o
	$(CC) $(CCFLAGS) -o bin/Monitor $(OBJS) build/$@.o $(LIBS)

build/%.o: src/%.cc
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o $@ $<
