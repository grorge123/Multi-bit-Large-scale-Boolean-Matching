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
int Abc_CommandPrintSymms( Abc_Frame_t * pAbc, int argc, char ** argv );
Abc_Ntk_t * Abc_FrameReadNtk( Abc_Frame_t * p );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


class ABCTool {
    Abc_Ntk_t *mainNtk{};
public:
    explicit ABCTool(string& path){
        init(path);
    }
    explicit ABCTool(AIG& cir){
        init(cir);
    }
    void init(string &path){
        char ** argv = new char *[2];
        char * pathArr = new char[path.size() + 1];
        strcpy(pathArr, path.c_str());
        argv[1] = pathArr;
        IoCommandReadAiger(pAbc, 2, argv);
        mainNtk = Abc_FrameReadNtk(pAbc);
    }
    void init(AIG &cir){
        ofstream file;
        string filename = "abcToolAIG.aag";
        string filename2 = "abcToolAIG.aig";
        string content = cir.getRaw();

        file.open(filename);
        if (file.is_open()) {
            aiger *aig = aiger_init();
            FILE *fp = nullptr;
            fp = fopen(filename.c_str(), "w+");
            fputs(content.c_str(), fp);
            fclose(fp);
            const char *err_msg = aiger_open_and_read_from_file(aig, filename.c_str());
#ifdef DBG
            if(err_msg != nullptr){
                cout << "[abcTool]ERROR: " << err_msg << endl;
                exit(1);
            }
#endif
            fp = fopen(filename2.c_str(), "w");
            aiger_write_to_file(aig, aiger_binary_mode, fp);
            fclose(fp);
            aiger_reset(aig);
            init(filename2);
        } else {
#ifdef DBG
            cout << "[abcTool] Error: cant not open file." << endl;
            exit(1);
#endif
        }
    }
    map<string, set<string> > funSupport(){
        Abc_Obj_t * pNode, * pNodeCi;
        map<string, set<string> > re;
        int i, v;
        Vec_Ptr_t * vSuppFun = Sim_ComputeFunSupp( mainNtk, 0 );
        Abc_NtkForEachCo( mainNtk, pNode, i )
            Abc_NtkForEachCi( mainNtk, pNodeCi, v ){
                if(Sim_SuppFunHasVar( vSuppFun, i, v ) != 0){
                    re[Abc_ObjName(pNode)].insert(Abc_ObjName(pNodeCi));
                    re[Abc_ObjName(pNodeCi)].insert(Abc_ObjName(pNode));
                }
            }
        return re;
    }
};
#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_ABCTOOL_H

