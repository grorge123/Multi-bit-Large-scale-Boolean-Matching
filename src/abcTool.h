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
typedef struct Vec_Int_t_       Vec_Int_t;
typedef struct Vec_Vec_t_       Vec_Vec_t;
typedef enum {
    ABC_NTK_NONE = 0,   // 0:  unknown
    ABC_NTK_NETLIST,    // 1:  network with PIs/POs, latches, nodes, and nets
    ABC_NTK_LOGIC,      // 2:  network with PIs/POs, latches, and nodes
    ABC_NTK_STRASH,     // 3:  structurally hashed AIG (two input AND gates with c-attributes on edges)
    ABC_NTK_OTHER       // 4:  unused
} Abc_NtkType_t;
typedef enum {
    ABC_FUNC_NONE = 0,  // 0:  unknown
    ABC_FUNC_SOP,       // 1:  sum-of-products
    ABC_FUNC_BDD,       // 2:  binary decision diagrams
    ABC_FUNC_AIG,       // 3:  and-inverter graphs
    ABC_FUNC_MAP,       // 4:  standard cell library
    ABC_FUNC_BLIFMV,    // 5:  BLIF-MV node functions
    ABC_FUNC_BLACKBOX,  // 6:  black box about which nothing is known
    ABC_FUNC_OTHER      // 7:  unused
} Abc_NtkFunc_t;
typedef struct Extra_BitMat_t_ Extra_BitMat_t;
typedef long ABC_INT64_T;
typedef ABC_INT64_T abctime;
typedef struct Nm_Man_t_ Nm_Man_t;
typedef struct Abc_Des_t_       Abc_Des_t;
typedef struct Abc_Cex_t_       Abc_Cex_t;
typedef struct Mem_Fixed_t_       Mem_Fixed_t;
typedef struct Mem_Step_t_       Mem_Step_t;
typedef struct Abc_ManTime_t_       Abc_ManTime_t;
struct Vec_Int_t_
{
    int              nCap;
    int              nSize;
    int *            pArray;
};
struct Abc_Ntk_t_
{
    // general information
    Abc_NtkType_t     ntkType;       // type of the network
    Abc_NtkFunc_t     ntkFunc;       // functionality of the network
    char *            pName;         // the network name
    char *            pSpec;         // the name of the spec file if present
    Nm_Man_t *        pManName;      // name manager (stores names of objects)
    // components of the network
    Vec_Ptr_t *       vObjs;         // the array of all objects (net, nodes, latches, etc)
    Vec_Ptr_t *       vPis;          // the array of primary inputs
    Vec_Ptr_t *       vPos;          // the array of primary outputs
    Vec_Ptr_t *       vCis;          // the array of combinational inputs  (PIs, latches)
    Vec_Ptr_t *       vCos;          // the array of combinational outputs (POs, asserts, latches)
    Vec_Ptr_t *       vPios;         // the array of PIOs
    Vec_Ptr_t *       vBoxes;        // the array of boxes
    Vec_Ptr_t *       vLtlProperties;
    // the number of living objects
    int nObjCounts[11];  // the number of objects by type
    int               nObjs;         // the number of live objs
    int               nConstrs;      // the number of constraints
    int               nBarBufs;      // the number of barrier buffers
    int               nBarBufs2;     // the number of barrier buffers
    // the backup network and the step number
    Abc_Ntk_t *       pNetBackup;    // the pointer to the previous backup network
    int               iStep;         // the generation number for the given network
    // hierarchy
    Abc_Des_t *       pDesign;       // design (hierarchical networks only)
    Abc_Ntk_t *       pAltView;      // alternative structural view of the network
    int               fHieVisited;   // flag to mark the visited network
    int               fHiePath;      // flag to mark the network on the path
    int               Id;            // model ID
    double            dTemp;         // temporary value
    // miscellaneous data members
    int               nTravIds;      // the unique traversal IDs of nodes
    Vec_Int_t         vTravIds;      // trav IDs of the objects
    Mem_Fixed_t *     pMmObj;        // memory manager for objects
    Mem_Step_t *      pMmStep;       // memory manager for arrays
    void *            pManFunc;      // functionality manager (AIG manager, BDD manager, or memory manager for SOPs)
    Abc_ManTime_t *   pManTime;      // the timing manager (for mapped networks) stores arrival/required times for all nodes
    void *            pManCut;       // the cut manager (for AIGs) stores information about the cuts computed for the nodes
    float             AndGateDelay;  // an average estimated delay of one AND gate
    int               LevelMax;      // maximum number of levels
    Vec_Int_t *       vLevelsR;      // level in the reverse topological order (for AIGs)
    Vec_Ptr_t *       vSupps;        // CO support information
    int *             pModel;        // counter-example (for miters)
    Abc_Cex_t *       pSeqModel;     // counter-example (for sequential miters)
    Vec_Ptr_t *       vSeqModelVec;  // vector of counter-examples (for sequential miters)
    Abc_Ntk_t *       pExdc;         // the EXDC network (if given)
    void *            pExcare;       // the EXDC network (if given)
    void *            pData;         // misc
    Abc_Ntk_t *       pCopy;         // copy of this network
    void *            pBSMan;        // application manager
    void *            pSCLib;        // SC library
    Vec_Int_t *       vGates;        // SC library gates
    Vec_Int_t *       vPhases;       // fanins phases in the mapped netlist
    char *            pWLoadUsed;    // wire load model used
    float *           pLutTimes;     // arrivals/requireds/slacks using LUT-delay model
    Vec_Ptr_t *       vOnehots;      // names of one-hot-encoded registers
    Vec_Int_t *       vObjPerm;      // permutation saved
    Vec_Int_t *       vTopo;
    Vec_Ptr_t *       vAttrs;        // managers of various node attributes (node functionality, global BDDs, etc)
    Vec_Int_t *       vNameIds;      // name IDs
    Vec_Int_t *       vFins;         // obj/type info
};
struct Abc_Obj_t_     // 48/72 bytes (32-bits/64-bits)
{
    Abc_Ntk_t *       pNtk;          // the host network
    Abc_Obj_t *       pNext;         // the next pointer in the hash table
    int               Id;            // the object ID
    unsigned          Type    :  4;  // the object type
    unsigned          fMarkA  :  1;  // the multipurpose mark
    unsigned          fMarkB  :  1;  // the multipurpose mark
    unsigned          fMarkC  :  1;  // the multipurpose mark
    unsigned          fPhase  :  1;  // the flag to mark the phase of equivalent node
    unsigned          fExor   :  1;  // marks AIG node that is a root of EXOR
    unsigned          fPersist:  1;  // marks the persistant AIG node
    unsigned          fCompl0 :  1;  // complemented attribute of the first fanin in the AIG
    unsigned          fCompl1 :  1;  // complemented attribute of the second fanin in the AIG
    unsigned          Level   : 20;  // the level of the node
    Vec_Int_t         vFanins;       // the array of fanins
    Vec_Int_t         vFanouts;      // the array of fanouts
    union { void *    pData;         // the network specific data
        int             iData; };      // (SOP, BDD, gate, equiv class, etc)
    union { void *    pTemp;         // temporary store for user's data
        Abc_Obj_t *     pCopy;         // the copy of this object
        int             iTemp;
        float           dTemp; };
};
struct Vec_Ptr_t_
{
    int              nCap;
    int              nSize;
    void **          pArray;
};
struct Vec_Vec_t_
{
    int              nCap;
    int              nSize;
    void **          pArray;
};
typedef struct Sym_Man_t_ Sym_Man_t;
struct Sym_Man_t_
{
    // info about the network
    Abc_Ntk_t *       pNtk;          // the network
    Vec_Ptr_t *       vNodes;        // internal nodes in topological order
    int               nInputs;
    int               nOutputs;
    // internal simulation information
    int               nSimWords;     // the number of bits in simulation info
    Vec_Ptr_t *       vSim;          // simulation info
    // support information
    Vec_Ptr_t *       vSuppFun;      // bit representation
    Vec_Vec_t *       vSupports;     // integer representation
    // symmetry info for each output
    Vec_Ptr_t *       vMatrSymms;    // symmetric pairs
    Vec_Ptr_t *       vMatrNonSymms; // non-symmetric pairs
    Vec_Int_t *       vPairsTotal;   // total pairs
    Vec_Int_t *       vPairsSym;     // symmetric pairs
    Vec_Int_t *       vPairsNonSym;  // non-symmetric pairs
    // temporary simulation info
    unsigned *        uPatRand;
    unsigned *        uPatCol;
    unsigned *        uPatRow;
    // temporary
    Vec_Int_t *       vVarsU;
    Vec_Int_t *       vVarsV;
    int               iOutput;
    int               iVar1;
    int               iVar2;
    int               iVar1Old;
    int               iVar2Old;
    // internal data structures
    int               nSatRuns;
    int               nSatRunsSat;
    int               nSatRunsUnsat;
    // pairs
    int               nPairsSymm;
    int               nPairsSymmStr;
    int               nPairsNonSymm;
    int               nPairsRem;
    int               nPairsTotal;
    // runtime statistics
    abctime           timeStruct;
    abctime           timeCount;
    abctime           timeMatr;
    abctime           timeSim;
    abctime           timeFraig;
    abctime           timeSat;
    abctime           timeTotal;
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
#define Sim_SuppStrHasVar(vSupps,pNode,v)     Sim_HasBit((unsigned*)(vSupps)->pArray[(pNode)->Id],(v))
static inline int Vec_IntSizeCopy( Vec_Int_t * p )
{
    return p->nSize;
}
static inline int Vec_IntEntryCopy( Vec_Int_t * p, int i )
{
    return p->pArray[i];
}
#define Vec_IntForEachEntry( vVec, Entry, i )                                               \
    for ( i = 0; (i < Vec_IntSizeCopy(vVec)) && (((Entry) = Vec_IntEntryCopy(vVec, i)), 1); i++ )
#define Vec_IntForEachEntryStart( vVec, Entry, i, Start )                                   \
    for ( i = Start; (i < Vec_IntSizeCopy(vVec)) && (((Entry) = Vec_IntEntryCopy(vVec, i)), 1); i++ )
char * Abc_ObjName( Abc_Obj_t * pObj );
Vec_Ptr_t * Sim_ComputeStrSupp( Abc_Ntk_t * pNtk );
Vec_Ptr_t * Sim_ComputeFunSupp( Abc_Ntk_t * pNtk, int fVerbose );
int IoCommandReadAiger( Abc_Frame_t * pAbc, int argc, char **argv );
Abc_Ntk_t * Abc_FrameReadNtk( Abc_Frame_t * p );
void Sim_UtilInfoFree( Vec_Ptr_t * p );
void Sym_ManStop( Sym_Man_t * p );
Sym_Man_t * Sim_ComputeTwoVarSymms2( Abc_Ntk_t * pNtk, int fVerbose );
int Extra_BitMatrixLookup1( Extra_BitMat_t * p, int i, int k );
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
    unordered_map<string, set<string>> funSupport();
    unordered_map<string, set<string>> strSupport();
    vector<pair<string, string>> calSymmetryPair(Extra_BitMat_t *pMat, Vec_Int_t *vSupport, Sym_Man_t *p);
    unordered_map<string, vector<pair<string, string>>> calSymmetry();
};
#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_ABCTOOL_H

