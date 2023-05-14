#ifdef __cplusplus
extern "C" {
#endif

// C struct declaration
struct solverResult{
    int satisfiable;
    int* input;
    int inputSize;
};
solverResult SAT_solver (char* fileName);
#ifdef __cplusplus
}
#endif