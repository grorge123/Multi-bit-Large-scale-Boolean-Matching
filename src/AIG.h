//
// Created by grorge on 5/15/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
#include "aiger.h"
#include "satsolver.h"
#include <cstring>
#include <map>
#include <set>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>

using namespace std;
class CNF;
class AIG {
    typedef pair<string,string> MP;
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
                                // verilog input name to order
    vector<bool> invMap; // AIG input order if is inverted
    map<string , set<string> > strSupport; // input Name to strSupport set search by recursive
    map<string , set<string> > abcStrSupport; // input Name to strSupport set search by abc
    map<string , set<string> > funSupport; // input Name to funSupport set
    map<string , vector<MP> > posSym;
    map<string , vector<MP> > negSym;
    void parseRaw();
    void recursiveFindSupport(int output, int now, vector<bool> &visit);
    bool recursiveGenerateOutput(int now, vector<int>& signal, vector<bool>& input);
    vector<string> zero, one;
    vector<pair<string,string>> wire; // <output name, input name>
    void recursiveFindStrSupport();
    void abcFindFunSupport();
    void abcFindStrSupport();
public:
    string cirName;
    AIG(){};
    AIG(const string &name, string cirName = "") : cirName(std::move(cirName)){
        aiger *input = aiger_init();
        const char *err_msg = aiger_open_and_read_from_file(input, name.c_str());
#ifdef DBG
        if(err_msg != nullptr){
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
    }
    const string &inputFromIndexToName(int index);
    const vector<string> &outputFromIndexToName(int idx);
    const string &fromOrderToName(int order);
    int fromOrderToIndex(int order) const;
    int fromNameToIndex(const string &name);
    int inputFromIndexToOrder(int idx);
    const vector<int> &outputFromIndexToOrder(int idx);
    int fromOrderToIndex(int order);
    int fromNameToOrder(string name);
    int getInputNum() const;
    int getOutputNum() const;
    bool isInput(int idx);
    bool isOutput(int idx);
    bool portExist(string name);
    bool portIsNegative(int order);
    const set<string> & getSupport(const string &name, int supType); // 0: funSuppose 1: abcStrSuppose 2: recurStrSuppose
    vector<bool> generateOutput(vector<bool> input);
    const string &getRaw();
    void changeName(const string& oldName, const string& newName);
    void erasePort(const vector<string>& nameList);
    const vector<string> &getZero() const;
    const vector<string> &getOne() const;
    void addNegativeOutput();
    void addFloatInput(const vector<string>& name);
    void invertGate(const string &name);
    void copyOutput(const string &origin, const string &newName, bool negative);
    void exportInput(const string &from, const string &to, bool negative);
    void setConstant(const string &origin, int val);
    void writeToAIGFile(const string &fileName);
    void modifyAIG();
    void Debug();
    void selfTest();
    void calSymmetry();
    vector<vector<string> > getHardSym();
};

void solveMiter(AIG &cir1, AIG &cir2, CNF &miter, AIG &miterAIG);

#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
