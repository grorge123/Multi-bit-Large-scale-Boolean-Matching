#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_SATSOLVER_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_SATSOLVER_H
    #ifdef __cplusplus
    extern "C" {
    #endif

    // C struct declaration
    struct solverResult{
        int satisfiable;
        int* input;
        int inputSize;
    };
    solverResult SAT_solver (const char* fileName);
    #ifdef __cplusplus
    }
    #endif

#endif