CC := g++
CFLAGS := -std=c++17 -lgtest -lpthread

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
EXECUTABLES := $(SOURCES:.cpp=.out)
LIB := ../lib/libabc.a ../lib/libkissat.a ../lib/aiger.o
obj_dir = ../obj
obj = $(wildcard $(obj_dir)/*.o)
obj := $(filter-out ../obj/main.o, $(obj))

all: run

$(EXECUTABLES):$(OBJECTS) preBuild
	$(CC) -o $@ $< $(obj) $(LIB) $(CFLAGS)

$(OBJECTS):$(SOURCES)
	$(CC) -c $@ $< $(CFLAGS)

preBuild:
	make -C "../" test

run: $(EXECUTABLES)
	@for file in $(EXECUTABLES); do \
		./$$file; \
	done


clean:
	rm -f *.o *.out