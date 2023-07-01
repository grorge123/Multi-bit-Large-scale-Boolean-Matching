//
// Created by grorge on 6/30/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_ABCTOOL_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_ABCTOOL_H
#include "utility.h"
#include "AIG.h"
#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif
typedef struct Abc_Frame_t_ Abc_Frame_t;
typedef struct Abc_Ntk_t_       Abc_Ntk_t;
typedef struct Abc_Obj_t_       Abc_Obj_t;
typedef struct Vec_Ptr_t_       Vec_Ptr_t;
struct Vec_Ptr_t_
{
    int              nCap;
    int              nSize;
    void **          pArray;
};
int Abc_NtkCiNum( Abc_Ntk_t * pNtk );
int Abc_NtkCoNum( Abc_Ntk_t * pNtk );
Abc_Obj_t * Abc_NtkCi( Abc_Ntk_t * pNtk, int i );
Abc_Obj_t * Abc_NtkCo( Abc_Ntk_t * pNtk, int i );
#define Abc_NtkForEachCo( pNtk, pCo, i )                                                           \
    for ( i = 0; (i < Abc_NtkCoNum(pNtk)) && (((pCo) = Abc_NtkCo(pNtk, i)), 1); i++ )
#define Abc_NtkForEachCi( pNtk, pCi, i )                                                           \
    for ( i = 0; (i < Abc_NtkCiNum(pNtk)) && (((pCi) = Abc_NtkCi(pNtk, i)), 1); i++ )
#define Sim_HasBit(p,i)      (((p)[(i)>>5]  & (1<<((i) & 31))) > 0)
#define Sim_SuppFunHasVar(vSupps,Output,v)    Sim_HasBit((unsigned*)(vSupps)->pArray[Output],(v))
char * Abc_ObjName( Abc_Obj_t * pObj );
Vec_Ptr_t *     Sim_ComputeFunSupp( Abc_Ntk_t * pNtk, int fVerbose );
int IoCommandReadAiger( Abc_Frame_t * pAbc, int argc, char **argv );
Abc_Ntk_t * Abc_FrameReadNtk( Abc_Frame_t * p );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


class ABCTool {
    Abc_Ntk_t *mainNtk{};
    FILE * stdoutSave();
    void stdoutRecovery(FILE *saveStdout);
public:
    explicit ABCTool(string& path){
        init(path);
    }
    explicit ABCTool(AIG& cir){
        init(cir);
    }
    void init(string &path);
    void init(AIG &cir);
    map<string, set<string> > funSupport();
};
#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_ABCTOOL_H

