#include "satsolver.h"
#include <stdio.h>
#include <stdlib.h>
int main (int argc, char **argv){
    for(int q = 0 ; q < 10 ; q++){
		printf("OLD: %s\n", argv[1]);
        solverResult res = SAT_solver(argv[1]);
        printf("%d\n", res.satisfiable);
        for(int i = 0 ; i < res.inputSize ; i++){
            printf("%d ", res.input[i]);
        }
        printf("\n");
		free(res.input);
    }

}
