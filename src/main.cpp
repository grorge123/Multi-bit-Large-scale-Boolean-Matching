#include "largeScale.h"
#include "parser.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>

extern Abc_Frame_t * pAbc;

int main (int argc, char *argv[]){
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();
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
    Abc_Stop();
}
