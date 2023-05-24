//
// Created by grorge on 5/15/23.
//

#include "AIG.h"
#include "utility.h"
#include <sstream>
#include <queue>

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
        tree[inpNum / 2].exist = true;
    }
    for(int i = 0 ; i < outputNum ; i++){
        int outNum;
        ss >> outNum;
        tree[outNum / 2].isOutput = true;
        indexMap.push_back(outNum / 2);
        indexMapInv[outNum / 2] = indexMap.size() - 1;
        invMap[outNum / 2] = outNum % 2;
        tree[outNum / 2].exist = true;
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
    while (!ss.eof() && idx < inputNum + outputNum){
        string a, b;
        ss >> a;
        if(a != "c"){
            ss >> b;
        }else{
            break;
        }
        string portName = cirName + b;
        nameMap[portName] = indexMap[idx];
        nameMapInv[indexMap[idx]] = portName;
        orderToName.push_back(portName);
        idx++;
    }
    return;
}

void AIG::findSupport() {
    for(int i = inputNum ; i < outputNum + inputNum; i++){
        int idx = indexMap[i];
        vector<bool> visit;
        if(idx == 0)continue;
        visit.resize(MAXIndex + 1);
        recursiveFindSupport(idx, idx, visit);
    }
}

void AIG::recursiveFindSupport(int output, int now, vector<bool> &visit) {
    if(visit[now])return;
    visit[now] = true;
    if(tree[now].isInput){
        support[now].insert(output);
        support[output].insert(now);
    }else{
        recursiveFindSupport(output, tree[now].l, visit);
        recursiveFindSupport(output, tree[now].r, visit);
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
        return (invMap[now] ? !input[indexMapInv[now]] : input[indexMapInv[now]]);
    }
    if(tree[now].l == -1 && tree[now].r == -1){
        return false;
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

const string &AIG::fromOrderToName(int idx) {
    return orderToName[idx];
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
        if(node.exist && !node.isInput){
            raw += to_string(node.inv[0] ? i * 2 + 1 : i * 2) + " " + to_string((node.inv[1] ? node.l * 2 + 1 : node.l * 2))
                    + " " + to_string((node.inv[2] ? node.r * 2 + 1 : node.r * 2)) + "\n";
        }
    }
    for(int q = 0 ; q < inputNum ; q++){
        raw += "i" + to_string(q) + " " + orderToName[q] + "\n";
    }
    for(int q = inputNum ; q < inputNum + outputNum; q++){
        raw += "o" + to_string(q - inputNum) + " " + orderToName[q] + "\n";
    }
    raw += "c\n";

    return raw;
}

void AIG::changeName(string oldName, string newName) {
    raw = "";
    for(unsigned int i = 0 ; i < orderToName.size() ; i++){
        if(orderToName[i] == oldName){
            orderToName[i] = newName;
            int tmpIdx = nameMap[oldName];
            nameMap[newName] = tmpIdx;
            nameMapInv[tmpIdx] = newName;
            nameMap.erase(oldName);
            break;
        }
    }
}

void AIG::erasePort(vector<string> nameList) {
    int removeAnd = 0;
    for(auto name : nameList){
        if(nameMap.find(name) == nameMap.end()){
            cout << "[AIG] ERROR: not found port" << endl;
        }
        if(tree[nameMap[name]].isInput){
            inputNum--;
        }else if(tree[nameMap[name]].isOutput){
            outputNum--;
            removeAnd++;
        }
        int nodeIdx = nameMap[name];
        int inputOrder = indexMapInv[nodeIdx];
        tree[nodeIdx].exist = false;
        nameMap.erase(name);
        nameMapInv.erase(nodeIdx);
        while (orderToName[inputOrder] != name)inputOrder--;
        indexMap.erase(indexMap.begin() + inputOrder);
        indexMapInv.erase(nodeIdx);
        orderToName.erase(orderToName.begin() + inputOrder);
        invMap.erase(nodeIdx);
    }
    vector<bool> exist;
    exist.resize(tree.size());
    queue<int> existQueue;
    for(int idx = inputNum ; idx < inputNum + outputNum ; idx++){
        if(indexMap[idx] != 0)
            existQueue.push(indexMap[idx]);
    }
    while (!existQueue.empty()){
        int now = existQueue.front();
        existQueue.pop();
        exist[now] = true;
        if(!tree[now].isInput){
            existQueue.push(tree[now].l);
            existQueue.push(tree[now].r);
        }
    }
    for(unsigned int idx = 1 ; idx < tree.size() ; idx++){
        if(tree[idx].exist && !exist[idx]){
            removeAnd++;
        }else if( !tree[idx].exist && exist[idx]){
            removeAnd--;
        }
        tree[idx].exist = exist[idx];
    }
    andNum -= removeAnd;
    support.clear();
    findSupport();
}

const string &AIG::fromIndexToName(int index) {
    return nameMapInv[index];
}
