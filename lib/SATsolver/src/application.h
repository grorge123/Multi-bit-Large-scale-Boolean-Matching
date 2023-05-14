#ifndef _application_h_INCLUDED
#define _application_h_INCLUDED
struct kissat;
typedef struct solverResult{
    int satisfiable;
    int* input;
    int inputSize;
};
typedef struct solverResult solverResult;
solverResult kissat_application (struct kissat *, int argc, char **argv);

#endif
