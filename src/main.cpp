#include "largeScale.h"
#include "TwoStep.h"
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
    lg = LargeScale(input, argv[2], 1);
    int lgMatch = lg.start();
    if(lgMatch == -1){
        return 0;
    }
    ts = TwoStep(input, argv[2]);
    ts.start();
    Abc_Stop();
    cout << "Optimize:" << allOptimize / 1000000 << endl;
    printStatistic();
}
