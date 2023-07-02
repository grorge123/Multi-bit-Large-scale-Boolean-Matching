//
// Created by grorge on 5/15/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
#include "aiger.h"
#include <string.h>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
class AIG {
    struct Node{
        bool inv[3] = {}; // self, l, r if invert
        int l = -1, r = -1;
        bool isInput = false;
        bool isOutput = false;
        bool exist = false;
    };
    vector<Node> tree;
    string raw;

private:
    int MAXIndex, inputNum, outputNum, latchNum, andNum;
    map<string, int> nameMap; // verilog input name to AIG Node index
    map<int, string> inputNameMapInv; // AIG Node index to verilog input name
    map<int, vector<string> > outputNameMapInv; // AIG Node index to verilog input name
    vector<int> indexMap; // AIG input order to AIG Node index
    map<int, int> inputIndexMapInv; // AIG Node index to AIG input order
    map<int, vector<int> > outputIndexMapInv; // AIG Node index to AIG input order
    vector<string> orderToName; // AIG input order to verilog name
    vector<bool> invMap; // AIG input order if is inverted
    map<int, set<int> > support; // AIG Node index to support set
    map<int, set<int> > funSupport; // AIG Node index to support set
    void parseRaw();
    void recursiveFindSupport(int output, int now, vector<bool> &visit);
    bool recursiveGenerateOutput(int now, vector<int>& signal, vector<bool>& input);
    void findSupport();
    void findFunSupport();
    vector<string> zero, one;
    vector<pair<string,string>> wire; // <output name, input name>

public:
    string cirName;
    AIG(){};
    AIG(string name, int supportType, string cirName = "") : cirName(cirName){
        aiger *input = aiger_init();
        const char *err_msg = aiger_open_and_read_from_file(input, name.c_str());
#ifdef DBG
        if(err_msg != NULL){
            cout << "[AIG]ERROR: " << err_msg << endl;
            exit(1);
        }
#endif
        ifstream ifs;
        ifs.open (name.c_str(), ios::binary );
        ifs.seekg (0, ios::end);
        int length = ifs.tellg();
        length = length * 8 + 1000;
        char* tmp = (char*)malloc(sizeof(char ) * length);
        aiger_mode mode = aiger_ascii_mode;
        int err = aiger_write_to_string(input, mode, tmp, length);
#ifdef DBG
        if(err != 1){
            cout << "[AIG]ERROR: AIG write error" << endl;
            exit(1);
        }
#endif
        raw = tmp;
        free(tmp);
        ifs.close();
        aiger_reset(input);
        parseRaw();
        if (supportType == 0 || supportType == 2) {
            findSupport();
        }
        if(supportType == 1 || supportType == 2){
            findFunSupport();
        }
    }
    const string &inputFromIndexToName(int index);
    const vector<string> &outputFromIndexToName(int idx);
    const string &fromOrderToName(int idx);
    int fromOrderToIndex(int order) const;
    int getInputNum();
    int getOutputNum();
    int getIdx(string name);
    int inputIdxToOrder(int idx);
    const vector<int> &outputIdxToOrder(int idx);
    bool isInput(int idx);
    set<string> getSupport(int idx);
    set<string> getSupport(string name);
    set<string> getFunSupport(int idx);
    set<string> getFunSupport(string name);
    vector<bool> generateOutput(vector<bool> input);
    const string &getRaw();
    void changeName(string oldName, string newName);
    void erasePort(vector<string> nameList);
    const vector<string> &getZero() const;
    const vector<string> &getOne() const;
    void addNegativeOutput();
    void invertGate(string &name);
    void Debug();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
