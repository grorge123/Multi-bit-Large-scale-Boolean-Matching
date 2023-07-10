#include "abcTool.h"

//
// Created by grorge on 7/1/23.
//
FILE * ABCTool::stdoutSave(){
    cout.flush();
    FILE *saveStdout = stdout;
    string resultPath = "stdoutOutput.txt";
    stdout = fopen(resultPath.c_str(), "w");
    return saveStdout;
}
void ABCTool::stdoutRecovery(FILE *saveStdout) {
    fflush(stdout);
    fclose(stdout);
    stdout = saveStdout;
}

void ABCTool::init(string &path){
    char ** argv = new char *[2];
    char * pathArr = new char[path.size() + 1];
    strcpy(pathArr, path.c_str());
    argv[1] = pathArr;
    FILE * saveStdout = stdoutSave();
    IoCommandReadAiger(pAbc, 2, argv);
    stdoutRecovery(saveStdout);
    mainNtk = Abc_FrameReadNtk(pAbc);
}

void ABCTool::init(AIG &cir) {
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

map<string, set<string> > ABCTool::funSupport(){
    Abc_Obj_t * pNode, * pNodeCi;
    map<string, set<string> > re;
    int i, v;
    FILE * saveStdout = stdoutSave();
    Vec_Ptr_t * vSuppFun = Sim_ComputeFunSupp( mainNtk, 0 );
    stdoutRecovery(saveStdout);
    Abc_NtkForEachCo( mainNtk, pNode, i )
        Abc_NtkForEachCi( mainNtk, pNodeCi, v ){
            if(Sim_SuppFunHasVar( vSuppFun, i, v ) != 0){
                string outputName = Abc_ObjName(pNode);
                string inputName = Abc_ObjName(pNodeCi);
                if(outputName == inputName)continue;
                re[outputName].insert(inputName);
                re[outputName].insert(inputName);
            }
        }
    return re;
}