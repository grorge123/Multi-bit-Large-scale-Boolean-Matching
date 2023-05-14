CC := g++
exe := main
#obj := main.o statistic.o evaluate.o utility.o
CFLAGS := -std=c++17 -O2 -Wall -Wextra
SANITIZE := -fsanitize=undefined -fsanitize=address
LINK := -lm -ldl -lreadline -lpthread
LIB := lib/libabc.a lib/libkissat.a
#INCLUDES :=

src_dir = ./src
obj_dir = ./obj

src = $(wildcard $(src_dir)/*.cpp)
obj = $(patsubst $(src_dir)/%.cpp, $(obj_dir)/%.o, $(src))

all:$(obj) $(LIB)
	mkdir -p $(obj_dir)
	$(CC) $(CFLAGS) -o $(exe) $(obj) $(LIB) $(LINK)

$(obj_dir)/%.o: $(src_dir)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

lib/libabc.a:
	$(MAKE) -C "lib/abc/" libabc.a
	mv lib/abc/libabc.a lib/

lib/libkissat.a:
	$(MAKE) -C "lib/SATsolver/build" libkissat.a
	mv lib/SATsolver/build/libkissat.a lib/
	$(MAKE) -C "lib/SATsolver/build" clean
clean:remove
	rm -rf $(obj) $(exe)

remove:
	rm -f nCir* output-*
