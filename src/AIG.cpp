//
// Created by grorge on 5/15/23.
//

#include "AIG.h"
#include "abcTool.h"
#include "utility.h"
#include "aigtocnf.h"
#include "CNF.h"
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
    int order = 0;
    while (!ss.eof() && order < inputNum + outputNum){
        string a, b;
        ss >> a;
        if(a != "c"){
            ss >> b;
        }else{
            break;
        }
        string portName = cirName + b;
        nameMap[portName] = indexMap[order];
        if(order < inputNum){
            inputNameMapInv[indexMap[order]] = portName;
        }else{
            outputNameMapInv[indexMap[order]].push_back(portName);
        }
        orderToName.push_back(portName);
        if(indexMap[order] == 0){
            if(invMap[order]){
                one.push_back(portName);
            }else{
                zero.push_back(portName);
            }
        }else if(order >= inputNum && tree[indexMap[order]].isInput){
            wire.emplace_back(portName, inputNameMapInv[indexMap[order]]);
        }
        order++;
    }

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
    funSupport.clear();
    ABCTool abcT(*this);
    map<string, set<string> > re = abcT.funSupport();
    for(auto &funPair : re){
        set<int> transfer;
        if(tree[fromNameToIndex(funPair.first)].isInput && fromNameToOrder(funPair.first) > inputNum){
            continue;
        }
        for(auto &port : funPair.second){
            transfer.insert(fromNameToIndex(port));
        }
        funSupport[fromNameToIndex(funPair.first)] = transfer;
    }
}


void AIG::recursiveFindSupport(int output, int now, vector<bool> &visit) {
    if(visit[now])return;
    visit[now] = true;
    if(now == 0)return;
    if(tree[now].isInput){
        if(now != output){
            support[now].insert(output);
            support[output].insert(now);
        }
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
set<string> AIG::getSupport(const string& name) {
    if(tree[fromNameToIndex(name)].isInput && fromNameToOrder(name) > inputNum){
        return set<string>{inputFromIndexToName(fromNameToIndex(name))};
    }
    return getSupport(fromNameToIndex(name));
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
    if(tree[fromNameToIndex(name)].isInput && fromNameToOrder(name) > inputNum){
        return set<string>{inputFromIndexToName(fromNameToIndex(name))};
    }
    return getFunSupport(fromNameToIndex(name));
}
int AIG::fromNameToIndex(string name) {
    return nameMap[name];
}

void AIG::Debug() {
    cout << "DEBUG:"<< (nameMap.find("@z") == nameMap.end()) << " " << nameMap.size() << endl;
    for(auto pair : nameMap){
        cout << "("<< pair.first << ", " << pair.second << ")" << " ";
    }
    cout << endl;
}

int AIG::getInputNum() {
    return inputNum;
}

const string &AIG::fromOrderToName(int order) {
#ifdef DBG
    if(static_cast<int>(orderToName.size()) <= order || order < 0){
        cout << "[AIG] Error: Can not transfer order(" << order << ") to name." << endl;
        exit(1);
    }
#endif
    return orderToName[order];
}

int AIG::getOutputNum() {
    return outputNum;
}

int AIG::inputFromIndexToOrder(int idx) {
#ifdef DBG
    if(inputIndexMapInv.find(idx) == inputIndexMapInv.end()){
        cout << "[AIG] Error: Can not transfer Index(" << idx << ") to order." << endl;
        exit(1);
    }
#endif
    return inputIndexMapInv[idx];
}

int AIG::fromOrderToIndex(int order) {
#ifdef DBG
    if(static_cast<int>(indexMap.size()) <= order || order < 0){
        cout << "[AIG] Error: Can not transfer order(" << order << ") to Index." << endl;
        exit(1);
    }
#endif
    return indexMap[order];
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
    int tmpOrder = -1;
#ifdef DBG
    if(nameMap.find(oldName) == nameMap.end()){
        cout << "[AIG] Error: Can not change " << oldName << " to " << newName << endl;
        exit(1);
    }
#endif
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
//TODO check wire erase input but output still exist
void AIG::erasePort(const vector<string>& nameList) {
    if(nameList.size() == 0)return;
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
#ifdef DBG
        if(!tree[now].exist){
            cout << "[AIG] Error: output port need path has been erased." << endl;
            exit(1);
        }
#endif
        if(!tree[now].isInput){
            if(now != 0){
                existQueue.push(tree[now].l);
                existQueue.push(tree[now].r);
            }
        }
    }
    //TODO fix don't care input will be remove in here
    for(unsigned int idx = 1 ; idx < tree.size() ; idx++){
        if(tree[idx].isInput)continue;
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
    findFunSupport();
}

const string &AIG::inputFromIndexToName(int index) {
#ifdef DBG
    if(index > MAXIndex || index < 0){
        cout << "[AIG] Error: Cant not transfer " << index << endl;
        exit(1);
    }
#endif
    return inputNameMapInv[index];
}

int AIG::fromNameToOrder(string name) {
    int idx = nameMap[name];
    if(tree[idx].isInput && inputNameMapInv[idx] == name){
        return inputIndexMapInv[idx];
    }else{
        for(auto order: outputIndexMapInv[idx]){
            if(orderToName[order] == name){
                return order;
            }
        }
    }
#ifdef DBG
    cout << "[AIG] Error: Cant not from Name(" << name << ") to find Order" << endl;
    exit(1);
#endif
    return -1;
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

bool AIG::portExist(string name) {
    return nameMap.find(name) != nameMap.end();
}

const vector<string> &AIG::outputFromIndexToName(int idx) {
    return outputNameMapInv[idx];
}

const vector<int> &AIG::outputFromIndexToOrder(int idx) {
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
            wire.emplace_back(newName, inputNameMapInv[indexMap[newOrder]]);
        }
    }
}

void AIG::addFloatInput(const vector<string>& name) {
    raw = "";
    indexMap.resize(indexMap.size() + name.size());
    orderToName.resize(orderToName.size() + name.size());
    invMap.resize(invMap.size() + name.size());
    for(auto i = inputNum + outputNum - 1 ; i >= inputNum ; i--){
        int originIdx = indexMap[i];
        indexMap[i + name.size()] = indexMap[i];
        for(auto &order : outputIndexMapInv[originIdx]){
            order += static_cast<int>(name.size());
        }
        orderToName[i + name.size()] = orderToName[i];
        invMap[i + name.size()] = invMap[i];
    }
    for(const auto & i : name){
        int newIdx = ++MAXIndex;
        nameMap[i] = newIdx;
        inputNameMapInv[newIdx] = i;
        inputIndexMapInv[newIdx] = inputNum; // AIG Node index to AIG input order
        indexMap[inputNum] = newIdx; // AIG input order to AIG Node index
        orderToName[inputNum] = i; // AIG input order to verilog name
        invMap[inputNum] = false; // AIG input order if is inverted
        inputNum++;
        tree.push_back(Node{{false,false,false}, -1, -1, true, false, true});
    }
}

void AIG::invertGate(const string &name) {
    raw = "";
#ifdef DBG
    if(nameMap.find(name) == nameMap.end()){
        cout << "[AIG] Error: invertGate can not find " << name << endl;
        exit(1);
    }
#endif
    int idx = nameMap[name];
    if(tree[idx].isInput && inputNameMapInv[idx] == name){
        for(auto &node : tree){
            if(node.l == idx){
                node.inv[1] = !node.inv[1];
            }
            if(node.r == idx){
                node.inv[2] = !node.inv[2];
            }
        }
    }else{
        for(auto &order: outputIndexMapInv[idx]){
            if(orderToName[order] == name){
                invMap[order] = !invMap[order];
                break;
            }
        }
    }
}

void AIG::copyOutput(const string &origin, const string &newName, const bool negative) {
    raw = "";
#ifdef DBG
    if(nameMap.find(origin) == nameMap.end()){
        cout << "[AIG] Error: Can not change " << origin << " to " << newName << endl;
        exit(1);
    }
#endif
    int originalIdx = nameMap[origin];
    nameMap[newName] = originalIdx;
    indexMap.push_back(originalIdx);
    int newOrder = indexMap.size() - 1;
    int originalOrder = -1;
    for(const auto &order :outputIndexMapInv[originalIdx]){
        if(orderToName[order] == origin){
            originalOrder = order;
            break;
        }
    }
#ifdef DBG
    if(originalOrder == -1){
        cout << "[AIG] Error: Can not find origin output." << endl;
        exit(1);
    }
#endif

    invMap.push_back((negative ? !invMap[originalOrder] : invMap[originalOrder]));
    outputIndexMapInv[originalIdx].push_back(newOrder);
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
        wire.emplace_back(newName, inputNameMapInv[indexMap[newOrder]]);
    }
}

void AIG::exportInput(const string &from, const string &to, bool negative) {
    raw = "";
#ifdef DBG
    if(nameMap.find(from) == nameMap.end() || nameMap.find(to) == nameMap.end()){
        cout << "[AIG] Error: Can not export " << from << " to " << to << endl;
        exit(1);
    }
#endif
    int fromIdx = nameMap[from];
    int toIdx = nameMap[to];
    for(auto &node : tree){
        if(node.l == toIdx){
            //assume input are all positive
#ifdef DBG
            if(tree[fromIdx].inv[0] != 0){
                cout << getRaw() << endl;
                cout << "[AIG]" << fromIdx << "(" << from << ")"<<"break assume;" << endl;
                exit(1);
            }
#endif
            node.l = fromIdx;
            if(negative)node.inv[1] = !node.inv[1];
        }
        if(node.r == toIdx){
#ifdef DBG
            if(tree[fromIdx].inv[1] != 0){
                cout << getRaw() << endl;
                cout << "[AIG]" << fromIdx << "(" << from << ")"<<"break assume;" << endl;
                exit(1);
            }
#endif
            node.r = fromIdx;
            if(negative)node.inv[2] = !node.inv[2];
        }
    }
    vector<string> ve;
    ve.emplace_back(to);
    erasePort(ve);
}

void AIG::setConstant(const string &origin, int val) {
    raw = "";
#ifdef DBG
    if(nameMap.find(origin) == nameMap.end()){
        exit(1);
    }
#endif
    int idx = nameMap[origin];
    for(auto &node : tree){
        if(node.l == idx){
            node.l = 0;
            if(val == 0){
                node.inv[1] = node.inv[1];
            }else{
                node.inv[1] = !node.inv[1];
            }
        }
        if(node.r == idx){
            node.r = 0;
            if(val == 0){
                node.inv[2] = node.inv[2];
            }else{
                node.inv[2] = !node.inv[2];
            }
        }
    }
    vector<string> ve;
    ve.emplace_back(origin);
    erasePort(ve);
}

void AIG::writeToAIGFile(const string &fileName) {
    string aagFileName = fileName.substr(0, fileName.size() - 4) + ".aag";
    aiger *aig = aiger_init();
    FILE *fp = nullptr;
    fp = fopen(aagFileName.c_str(), "w+");
    fputs(getRaw().c_str(), fp);
    fclose(fp);
    const char *err_msg = aiger_open_and_read_from_file(aig, aagFileName.c_str());
#ifdef DBG
    if(err_msg != nullptr){
        cout << "[AIG:writeToAIGFile]ERROR: file:" << fileName << " msg:" << err_msg << endl;
        exit(1);
    }
#endif
    fp = fopen(fileName.c_str(), "w");
    aiger_write_to_file(aig, aiger_binary_mode, fp);
    fclose(fp);
    aiger_reset(aig);
}

bool AIG::portIsNegative(int order) {
    return invMap[order];
}

void solveMiter(AIG &cir1, AIG &cir2, CNF &miter, AIG &miterAIG) {
    string savePath1 = "AIGSave1.aig";
    string savePath2 = "AIGSave2.aig";
    cir1.writeToAIGFile(savePath1);
    cir2.writeToAIGFile(savePath2);

    //TODO optimize abc command
    string abcCmd = "miter " + savePath1 + " " + savePath2 + "; write_aiger -s miter.aig;";
    string resultPath = "stdoutOutput.txt";
    cout.flush();
    FILE *saveStdout = stdout;
    stdout = fopen(resultPath.c_str(), "a");
    if (stdout != NULL) {
        if (Cmd_CommandExecute(pAbc, abcCmd.c_str())){
#ifdef DBG
            cout << "[AIG] ERROR:Cannot execute command \"" << abcCmd << "\".\n";
            exit(1);
#endif
        }
        fflush(stdout);
        fclose(stdout);
        stdout = saveStdout;
    } else {
#ifdef DBG
        cout << "[AIG] ERROR:Can't write file:" << resultPath << endl;
        exit(1);
#endif
    }
    char miterAIGFileName[]{"miter.aig"};
    char miterCNFFileName[]{"miter.cnf"};
    aigtocnf(miterAIGFileName, miterCNFFileName);
    miterAIG = AIG("miter.aig", 3);
    miter = CNF(miterAIG);
    miter.solve();
//    solverResult result = SAT_solver(miterCNF);
//
//    return result;
}
