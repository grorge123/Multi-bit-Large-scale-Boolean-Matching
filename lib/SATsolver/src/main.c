
#include "solver.h"
#include <stdio.h>
int main (int argc, char **argv){
    for(int q = 0 ; q < 10 ; q++){
        solverResult res = SAT_solver(argv[2]);
        printf("%d\n", res.satisfiable);
        for(int i = 0 ; i < res.inputSize ; i++){
            printf("%d ", res.input[i]);
        }
        printf("\n");
    }

}
