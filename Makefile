CC := g++
exe := main
#obj := main.o statistic.o evaluate.o utility.o
CFLAGS := -std=c++17 -O2 -Wall -Wextra
SANITIZE := -fsanitize=undefined -fsanitize=address
LINK := -lm -ldl -lreadline
LIB := lib/libabc.a lib/libkissat.a lib/aiger.o
#INCLUDES :=

src_dir = ./src
obj_dir = ./obj

src = $(wildcard $(src_dir)/*.cpp)
header = $(wildcard $(src_dir)/*.h)
obj = $(patsubst $(src_dir)/%.cpp, $(obj_dir)/%.o, $(src))

all:$(obj) $(LIB) $(header) initDir
	$(CC) $(CFLAGS) -o $(exe) $(obj) $(LIB)

initDir:
	mkdir -p $(obj_dir)

$(obj_dir)/%.o: $(src_dir)/%.cpp $(header)
	$(CC) $(CFLAGS) -c $< -o $@

lib/libabc.a:
	$(MAKE) -C "lib/abc/" libabc.a ABC_USE_NO_PTHREADS=1 ABC_USE_NO_READLINE=1
	mv lib/abc/libabc.a lib/

lib/libkissat.a:
	$(MAKE) -C "lib/SATsolver/build" libkissat.a
	mv lib/SATsolver/build/libkissat.a lib/
	$(MAKE) -C "lib/SATsolver/build" clean

lib/aiger.o:
	gcc -c lib/aiger/aiger.c -o lib/aiger.o

test:$(obj) $(LIB) $(header)
	echo $(header)

clean:
	rm -rf $(obj) $(exe)
run: all
	./main input.txt output.txt
	rm -f save* miter* tmp* stdoutOutput.txt *.v.aig

