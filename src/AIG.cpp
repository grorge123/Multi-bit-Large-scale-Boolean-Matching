//
// Created by grorge on 5/15/23.
//

#include "AIG.h"
#include "utility.h"
#include <sstream>

void AIG::parseRaw() {
    stringstream ss;
    ss << raw;
    string tag;
    ss >> tag >> MAXIndex >> inputNum >> latchNum >> outputNum >> andNum;
    if(tag != "aag" || latchNum != 0){
        cout << "ERROR: Can not parse AIG." << endl;
        exit(1);
    }
    tree.resize(MAXIndex+1);
    for(int i = 0 ; i < inputNum ; i++){
        int inpNum;
        ss >> inpNum;
        tree[inpNum / 2].isInput = true;

        indexMap.push_back(inpNum / 2);
        indexMapInv[inpNum / 2] = indexMap.size() - 1;
        invMap[inpNum / 2] = inpNum % 2;
    }
    for(int i = 0 ; i < outputNum ; i++){
        int outNum;
        ss >> outNum;
        indexMap.push_back(outNum / 2);
        indexMapInv[outNum / 2] = indexMap.size() - 1;
        invMap[outNum / 2] = outNum % 2;
    }
    for(int i = 0 ; i < andNum ; i++){
        int out, l, r;
        ss >> out >> l >> r;
        int idx = out / 2;
        tree[idx].l = l / 2;
        tree[idx].r = r / 2;
        tree[idx].inv[0] = out % 2;
        tree[idx].inv[1] = l % 2;
        tree[idx].inv[2] = r % 2;
    }
    int idx = 0;
    while (true){
        string a, b;
        ss >> a;
        if(a != "c"){
            ss >> b;
        }else{
            break;
        }
        nameMap[b] = indexMap[idx];
        nameMapInv[indexMap[idx]] = b;
        indexToName.push_back(b);
        idx++;
    }
    return;
}

void AIG::findSupport() {
    for(int i = inputNum ; i < outputNum + inputNum; i++){
        int idx = indexMap[i];
        recursiveFindSupport(idx, idx);
    }
}

void AIG::recursiveFindSupport(int output, int now) {
    if(tree[now].isInput){
        support[now].insert(output);
        support[output].insert(now);
    }else{
        recursiveFindSupport(output, tree[now].l);
        recursiveFindSupport(output, tree[now].r);
    }
}

vector<bool> AIG::generateOutput(vector<bool> input) {
    vector<bool> output;
    vector<int> signal;
    signal.resize(MAXIndex + 1, -1);
    output.resize(outputNum);
    for(int i = inputNum ; i < outputNum + inputNum ; i++){
        signal[indexMap[i]] = recursiveGenerateOutput(indexMap[i], signal, input);
        output[i - inputNum] = (invMap[indexMap[i]] ? !signal[indexMap[i]] : signal[indexMap[i]]);
    }
    return output;
}

bool AIG::recursiveGenerateOutput(int now, vector<int> &signal, vector<bool> &input) {
    if(tree[now].isInput){
        return (invMap[now] ? !input[now - 1] : input[now - 1]);
    }
    if(signal[tree[now].l] == -1){
        signal[tree[now].l] = recursiveGenerateOutput(tree[now].l, signal, input);
    }
    if(signal[tree[now].r] == -1){
        signal[tree[now].r] = recursiveGenerateOutput(tree[now].r, signal, input);
    }
    bool lvalue = (tree[now].inv[1] ? !signal[tree[now].l] : signal[tree[now].l]);
    bool rvalue = (tree[now].inv[2] ? !signal[tree[now].r] : signal[tree[now].r]);
    return (tree[now].inv[0] ? !(lvalue & rvalue) : (lvalue & rvalue));
}

set<string> AIG::getSupport(int idx) {
    set<string> re;
    for(auto i : support[idx]){
        re.insert(nameMapInv[i]);
    }
    return re;
}
set<string> AIG::getSupport(string name) {
    return getSupport(getIdx(name));
}

int AIG::getIdx(string name) {
    return nameMap[name];
}

void AIG::Debug() {
    for(auto i : support){
        cout << "Key:" << i.first << endl << "Value:";
        for(auto q : i.second){
            cout << q << ' ';
        }
        cout << endl;
    }
    for(auto i : nameMap){
        cout << i.first << ' ' << i.second << endl;
    }
    for(auto i: nameMapInv){
        cout << i.first << ' ' << i.second <<endl;
    }
}

int AIG::getInputNum() {
    return inputNum;
}

string AIG::fromIndexToName(int idx) {
    return indexToName[idx];
}

int AIG::getOutputNum() {
    return outputNum;
}
