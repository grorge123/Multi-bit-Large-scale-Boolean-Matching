CC := g++
CFLAGS := -std=c++17 -lgtest -lpthread

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
EXECUTABLES := $(SOURCES:.cpp=.out)

all: run

$(EXECUTABLES):
	$(CC) $@.cpp -o $@ $(CFLAGS)

run: $(EXECUTABLES)
	@for file in $(EXECUTABLES); do \
		./$$file; \
	done


clean:
	rm -f *.o *.out