//
// Created by grorge on 5/15/23.
//

#include "AIG.h"
#include "abcTool.h"
#include "utility.h"
#include <sstream>
#include <queue>

void AIG::parseRaw() {
    stringstream ss;
    ss << raw;
    string tag;
    ss >> tag >> MAXIndex >> inputNum >> latchNum >> outputNum >> andNum;
#ifdef DBG
    if(tag != "aag" || latchNum != 0){
        cout << "ERROR: Can not parse AIG." << endl;
        exit(1);
    }
#endif
    tree.resize(MAXIndex+1);
    for(int i = 0 ; i < inputNum ; i++){
        int inpNum;
        ss >> inpNum;
        tree[inpNum / 2].isInput = true;
        indexMap.push_back(inpNum / 2);
        inputIndexMapInv[inpNum / 2] = indexMap.size() - 1;
        invMap.push_back(inpNum % 2);
        tree[inpNum / 2].exist = true;
    }
    for(int i = 0 ; i < outputNum ; i++){
        int outNum;
        ss >> outNum;

        indexMap.push_back(outNum / 2);
        invMap.push_back(outNum % 2);
        outputIndexMapInv[outNum / 2].push_back(indexMap.size() - 1);
        tree[outNum / 2].isOutput = true;
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
        if(idx < inputNum){
            inputNameMapInv[indexMap[idx]] = portName;
        }else{
            outputNameMapInv[indexMap[idx]].push_back(portName);
        }
        orderToName.push_back(portName);
        if(indexMap[idx] == 0){
            if(invMap[idx]){
                one.push_back(portName);
            }else{
                zero.push_back(portName);
            }
        }else if(idx >= inputNum && tree[indexMap[idx]].isInput){
            wire.push_back(pair<string,string>(portName, inputNameMapInv[indexMap[idx]]));
        }
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

void AIG::findFunSupport() {
    ABCTool abcT(*this);
    map<string, set<string> > re = abcT.funSupport();
    for(auto &funPair : re){
        set<int> transfer;
        for(auto &port : funPair.second){
            transfer.insert(getIdx(port));
        }
        funSupport[getIdx(funPair.first)] = transfer;
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
        output[i - inputNum] = (invMap[i] ? !signal[indexMap[i]] : signal[indexMap[i]]);
    }
    return output;
}

bool AIG::recursiveGenerateOutput(int now, vector<int> &signal, vector<bool> &input) {
    if(tree[now].isInput){
        return (invMap[inputIndexMapInv[now]] ? !input[inputIndexMapInv[now]] : input[inputIndexMapInv[now]]);
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
    for(auto &i : support[idx]){
        if(tree[i].isInput){
            re.insert(inputNameMapInv[i]);
        }else{
            for(auto &outputName : outputNameMapInv[i]){
                re.insert(outputName);
            }
        }
    }
    return re;
}
set<string> AIG::getSupport(string name) {
    return getSupport(getIdx(name));
}
set<string> AIG::getFunSupport(int idx){
    set<string> re;
    for(auto &i : funSupport[idx]){
        if(tree[i].isInput){
            re.insert(inputNameMapInv[i]);
        }else{
            for(auto &outputName : outputNameMapInv[i]){
                re.insert(outputName);
            }
        }
    }
    return re;
}
set<string> AIG::getFunSupport(string name) {

    return getFunSupport(getIdx(name));
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

int AIG::inputIdxToOrder(int idx) {
    return inputIndexMapInv[idx];
}

const string &AIG::getRaw() {
//    if(raw.size() != 0)return raw;
    raw = "aag " + to_string(MAXIndex) + " " + to_string(inputNum) + " 0 " + to_string(outputNum) + " " +
            to_string(andNum) + "\n";
    for(int i = 0 ; i < inputNum + outputNum ; i++){
        raw += (invMap[i] ? to_string(indexMap[i] * 2 + 1) : to_string(indexMap[i] * 2)) + "\n";
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
    int tmpOrder;
    int tmpIdx = nameMap[oldName];
    nameMap[newName] = tmpIdx;
    if(tree[tmpIdx].isInput && inputNameMapInv[tmpIdx] == oldName){
        inputNameMapInv[tmpIdx] = newName;
        tmpOrder = inputIndexMapInv[tmpIdx];
    }else{
        for(auto &order : outputIndexMapInv[tmpIdx]){
            if(orderToName[order] == oldName){
                tmpOrder = order;
            }
        }
        for(auto &outputName : outputNameMapInv[tmpIdx]){
            if(outputName == oldName){
                outputName = newName;
                break;
            }
        }
    }
    orderToName[tmpOrder] = newName;
    nameMap.erase(oldName);

}

void AIG::erasePort(vector<string> nameList) {
    int removeAnd = 0;
    for(auto &name : nameList){
#ifdef DBG
        if(nameMap.find(name) == nameMap.end()){
            cout << "[AIG] ERROR: not found port" << endl;
        }
#endif
        unsigned int nodeIdx = nameMap[name];
        if(tree[nodeIdx].isInput && name == inputNameMapInv[nodeIdx]){
            inputNum--;
            unsigned int inputOrder = min(inputIndexMapInv[nodeIdx], (int)orderToName.size() - 1);
            tree[nodeIdx].exist = false;
            nameMap.erase(name);
            inputNameMapInv.erase(nodeIdx);
            while (orderToName[inputOrder] != name){
#ifdef DBG
                if(inputOrder == 0){
                    cout << "[AIG] ERROR: Can not found input Name" << "(" << name << ")" << "from orderToName" << endl;
                    exit(1);
                }
#endif
                inputOrder--;
            }
            indexMap.erase(indexMap.begin() + inputOrder);
            inputIndexMapInv.erase(nodeIdx);
            orderToName.erase(orderToName.begin() + inputOrder);
            invMap.erase(invMap.begin() + inputOrder);
        }else{
            outputNum--;
            unsigned int inputOrder = 0;
            for(auto &order : outputIndexMapInv[nodeIdx]){
                inputOrder = max((int)inputOrder, order);
            }
            inputOrder = min((unsigned int)orderToName.size() - 1, inputOrder);
            while (orderToName[inputOrder] != name){
#ifdef DBG
                if(inputOrder == 0){
                    cout << "[AIG] ERROR: Can not found input Name" << "(" << name << ")" << "from orderToName" << endl;
                    exit(1);
                }
#endif
                inputOrder--;
            }
            nameMap.erase(name);
            indexMap.erase(indexMap.begin() + inputOrder);
            orderToName.erase(orderToName.begin() + inputOrder);
            invMap.erase(invMap.begin() + inputOrder);
            for(auto it = outputNameMapInv[nodeIdx].begin() ; it != outputNameMapInv[nodeIdx].end() ; it++){
                if(*it == name){
                    outputNameMapInv[nodeIdx].erase(it);
                    break;
                }
            }
            for(auto it = outputIndexMapInv[nodeIdx].begin() ; it != outputIndexMapInv[nodeIdx].end() ; it++){
                if(*it == (int)inputOrder){
                    outputIndexMapInv[nodeIdx].erase(it);
                    break;
                }
            }
        }
    }
    inputIndexMapInv.clear();
    outputIndexMapInv.clear();
    for(unsigned int order = 0 ; order < indexMap.size() ; order++){
        if((int)order < inputNum){
            inputIndexMapInv[indexMap[order]] = order;
        }else{
            outputIndexMapInv[indexMap[order]].push_back(order);
        }
    }

    vector<bool> exist;
    exist.resize(tree.size());
    queue<int> existQueue;
    for(int idx = inputNum ; idx < inputNum + outputNum ; idx++){
        if(indexMap[idx] != 0){
            existQueue.push(indexMap[idx]);
        }
    }
    while (!existQueue.empty()){
        int now = existQueue.front();
        existQueue.pop();
        if(exist[now])continue;
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

const string &AIG::inputFromIndexToName(int index) {
    return inputNameMapInv[index];
}

int AIG::fromOrderToIndex(int order) const {
    return indexMap[order];
}

const vector<string> &AIG::getZero() const {
    return zero;
}

const vector<string> &AIG::getOne() const {
    return one;
}

bool AIG::isInput(int idx) {
    return tree[idx].isInput;
}

const vector<string> &AIG::outputFromIndexToName(int idx) {
    return outputNameMapInv[idx];
}

const vector<int> &AIG::outputIdxToOrder(int idx) {
    return outputIndexMapInv[idx];
}

void AIG::addNegativeOutput() {
    int oldNum = inputNum + outputNum;
    for(int i = inputNum ; i < oldNum ; i++){
        int newIndex = indexMap[i];
        indexMap.push_back(newIndex);
        int newOrder = indexMap.size() - 1;
        invMap.push_back(!invMap[i]);
        outputIndexMapInv[newIndex].push_back(newOrder);
        string newName = orderToName[i] + '\'';
        nameMap[newName] = indexMap[newOrder];
        outputNameMapInv[indexMap[newOrder]].push_back(newName);
        orderToName.push_back(newName);
        outputNum++;
        if(indexMap[newOrder] == 0){
            if(invMap[newOrder]){
                one.push_back(newName);
            }else{
                zero.push_back(newName);
            }
        }else if(tree[indexMap[newOrder]].isInput){
            wire.push_back(pair<string,string>(newName, inputNameMapInv[indexMap[newOrder]]));
        }
    }
}

void AIG::invertGate(string &name) {
    int idx = nameMap[name];
    if(tree[idx].isInput && inputNameMapInv[idx] == name){
        invMap[inputIndexMapInv[idx]] = !invMap[inputIndexMapInv[idx]];
    }else{
        for(auto &order: outputIndexMapInv[idx]){
            if(orderToName[order] == name){
                invMap[order] = !invMap[order];
                break;
            }
        }
    }
}



