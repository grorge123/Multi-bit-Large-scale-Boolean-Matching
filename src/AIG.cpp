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
        tree[idx].exist = true;
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
    for(int i = 0 ; i < inputNum + outputNum ; i++){
        cout << i << ' ' << invMap[i] << endl;
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

int AIG::idxToOrder(int idx) {
    return indexMapInv[idx];
}

const string &AIG::getRaw() {
//    if(raw.size() != 0)return raw;
    raw = "aag " + to_string(MAXIndex) + " " + to_string(inputNum) + " 0 " + to_string(outputNum) + " " +
            to_string(andNum) + "\n";
    for(int i = 0 ; i < inputNum + outputNum ; i++){
        raw += (invMap[indexMap[i]] ? to_string(indexMap[i] * 2 + 1) : to_string(indexMap[i] * 2)) + "\n";
    }
    for(int i = 1 ; i <= MAXIndex ; i++){
        Node node = tree[i];
        if(node.exist){
            raw += to_string(node.inv[0] ? i * 2 + 1 : i * 2) + " " + to_string((node.inv[1] ? node.l * 2 + 1 : node.l * 2))
                    + " " + to_string((node.inv[2] ? node.r * 2 + 1 : node.r * 2)) + "\n";
        }
    }
    for(int q = 0 ; q < inputNum ; q++){
        raw += "i" + to_string(q) + " " + indexToName[q] + "\n";
    }
    for(int q = inputNum ; q < inputNum + outputNum; q++){
        raw += "o" + to_string(q - inputNum) + " " + indexToName[q] + "\n";
    }
    raw += "c\n";

    return raw;
}

void AIG::changeName(string oldName, string newName) {
    raw = "";
    for(unsigned int i = 0 ; i < indexToName.size() ; i++){
        if(indexToName[i] == oldName){
            indexToName[i] = newName;
            int tmpIdx = nameMap[oldName];
            nameMap[newName] = tmpIdx;
            nameMapInv[tmpIdx] = newName;
            nameMap.erase(oldName);
            break;
        }
    }
}
