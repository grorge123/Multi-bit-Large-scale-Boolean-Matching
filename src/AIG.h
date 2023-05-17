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

using namespace std;
class AIG {
    struct node{
        bool inv[3] = {}; // self, l, r if invert
        int l = -1, r = -1;
        bool isInput = false;
    };
    vector<node> tree;
    string raw;
    int MAXIndex, inputNum, outputNum, latchNum, andNum;
    map<string, int> nameMap; // verilog input name to AIG node index
    map<int, string> nameMapInv; // node index to verilog input name
    vector<int> indexMap; // AIG input order to AIG node index
    vector<string> indexToName; // AIG input order to verilog name
    map<int, int> indexMapInv; // AIG node index to AIG input order
    map<int, bool> invMap; // AIG node index if is invert
    map<int, set<int> > support;
    void parseRaw();
    void recursiveFindSupport(int output, int now);
    bool recursiveGenerateOutput(int now, vector<int>& signal, vector<bool>& input);
    void findSupport();
public:
    AIG(string name){
        aiger *input = aiger_init();
        const char *err_msg = aiger_open_and_read_from_file(input, name.c_str());
        if(err_msg != NULL){
            cout << "[AIG]ERROR: " << err_msg << endl;
            exit(1);
        }
        const int MAXLen = 100000;
        char tmp[MAXLen] = {};
        aiger_mode mode = aiger_ascii_mode;
        int err = aiger_write_to_string(input, mode, tmp, MAXLen);
        if(err != 1){
            cout << "[AIG]ERROR: AIG write error" << endl;
            exit(1);
        }
        raw = tmp;
        aiger_reset(input);
        parseRaw();
        findSupport();
        return;
    }
    string fromIndexToName(int idx);
    int getInputNum();
    int getOutputNum();
    int getIdx(string name);
    set<string> getSupport(int idx);
    set<string> getSupport(string name);
    vector<bool> generateOutput(vector<bool> input);
    void Debug();
};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
