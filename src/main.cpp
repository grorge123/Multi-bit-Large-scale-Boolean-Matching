#include "largeScale.h"
#include "parser.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>

extern Abc_Frame_t * pAbc;
extern LargeScale lg;
int main (int argc, char *argv[]){
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();
#ifdef DBG
    if(argc != 3){
        cout << "[main] ERROR: wrong argument number!\n";
        exit(1);
    }
#endif
    InputStructure input = parseInput(argv[1]);
    lg = LargeScale(input, argv[2]);
    int lgMatch = lg.start();
    if(lgMatch == -1){
        return 0;
    }
    Abc_Stop();
}
