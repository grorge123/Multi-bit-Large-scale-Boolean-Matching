CC := g++
CFLAGS := -std=c++17 -lgtest -lpthread

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
EXECUTABLES := $(SOURCES:.cpp=.out)
LIB := ../lib/libabc.a ../lib/libkissat.a ../lib/aiger.o
obj_dir = ../obj
src_dir = ../src
obj = $(wildcard $(obj_dir)/*.o)
obj := $(filter-out ../obj/main.o, $(obj))
header = $(wildcard $(src_dir)/*.h)

all: $(EXECUTABLES)

%.out: %.o preBuild
	$(CC) -o $@ $(patsubst %.out, %.o, $@) $(obj) $(LIB) $(CFLAGS)
	./$@

$(EXECUTABLES): $(OBJECTS) preBuild
	$(CC) -o $@ $(patsubst %.out, %.o, $@) $(obj) $(LIB) $(CFLAGS)
	./$@


%.o : %.cpp $(header)
	$(CC) -c $@ $< $(CFLAGS)

preBuild:
	make -C "../" test

clean:
	rm -f *.o *.out