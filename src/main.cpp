#include "largeScale.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>



int main (int argc, char *argv[]){
    if(argc != 3){
        cout << "[main] ERROR: wrong argument number!\n";
        exit(1);
    }
    InputStructure input = parseInput(argv[1]);
    LargeScale lg(input, argv[2]);
    int lgMatch = lg.start();
    if(lgMatch == -1){
        return 0;
    }
}
