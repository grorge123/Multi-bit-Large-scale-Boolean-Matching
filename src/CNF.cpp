//
// Created by grorge on 6/28/23.
//
#include <sstream>
#include "CNF.h"
#include "satsolver.h"
#include "aigtocnf.h"
void CNF::readFromAIG(AIG &aig) {
    string fileName = "tmpCNFConstructor";
    string aigContent = aig.getRaw();
    ofstream ofs;
    ofs.open(fileName + ".aig");
    ofs << aigContent;
    ofs.close();
    aigtocnf((fileName + ".aig").c_str(), (fileName + ".cnf").c_str());
    // read another info
    readFromFile(fileName + ".cnf");

    ifstream ifs;
    ifs.open(fileName + ".cnf");
    string input;
#ifdef DBG
    for(int i = 0 ; i < aig.getInputNum() + aig.getOutputNum() ; i++){
        DC.insert(aig.fromOrderToName(i));
    }
#endif
    inv.resize(maxIdx);
    while (getline(ifs, input)){
        if(input[0] != 'c')break;
        stringstream ss;
        ss.clear();
        ss << input;
        ss >> input;
        int aigIdx = -1, cnfIdx = -1;
        ss >> aigIdx >> input >> cnfIdx;
#ifdef DBG
            if(aigIdx % 2 != 0){
                cout << "[CNF] Break aigIdx not negative assume" << endl;
                exit(1);
            }
#endif
        aigIdx /= 2;
        if(aig.isInput(aigIdx)){
            int order = aig.inputFromIndexToOrder(aigIdx);
            if(aig.portIsNegative(order)){
                inv[cnfIdx];
            }
#ifdef DBG
            DC.erase(aig.inputFromIndexToName(aigIdx));
#endif
            varMap.insert(pair<string, int> (aig.inputFromIndexToName(aigIdx), cnfIdx));
        }
        for(const auto& name : aig.outputFromIndexToName(aigIdx)){
#ifdef DBG
            DC.erase(name);
#endif
//            varMap.insert(pair<string, int> (name, cnfIdx));
        }
    }
    ifs.close();

}


void CNF::readFromFile(string inputPath) {
    ifstream ifs;
    ifs.open(inputPath);
    string input;
    while (getline(ifs, input)){
        if(input[0] == 'c')continue;
        stringstream ss;
        ss << input;
        if(input[0] == 'p'){
            ss >> input; // read p
            ss >> input; // read cnf
            ss >> maxIdx;
            int clauseNum;
            ss >> clauseNum;
            clauses.reserve(clauseNum);
        }else{
            vector<int> ve;
            int now;
            while (ss >> now){
                if(now == 0) break;
                ve.push_back(now);
            }
            clauses.push_back(ve);
        }
    }
    ifs.close();
}

string CNF::getRaw() {
    string raw;
    string saveSpace(10, ' ');
    raw += "p cnf " + to_string(maxIdx) + " " + to_string(clauses.size()) + saveSpace + '\n';
    for(const auto& clause : clauses){
        for(auto var : clause){
            raw += to_string(var) + " ";
        }
        raw += "0\n";
    }
    return raw;
}

void CNF::combine(const CNF &a) {
    checkSatisfied = false;
    for(auto mapTable : a.varMap){
#ifdef DBG
        if(varMap.find(mapTable.first) != varMap.end()){
            cout << "[CNF] ERROR: Multiple mapping" << endl;
        }
#endif
        varMap[mapTable.first] = mapTable.second + maxIdx;
    }
    for(auto clause : clauses){
        vector<int> ve;
        for(auto var : clause){
            int newVar = (abs(var) + maxIdx ) * (var > 0 ? 1 : -1);
            ve.push_back(newVar);
        }
        clauses.push_back(ve);
    }
    maxIdx += a.maxIdx;
}

bool CNF::solve() {
    //TODO finish checkSatisfied;
    if(checkSatisfied){
        return satisfiable;
    }else{
        string fileName = "CNFSatisfied.cnf";
        string content = getRaw();
        ofstream ofs;
        ofs.open(fileName);
        ofs << content;
        ofs.close();
        char * miterCNF = new char[fileName.size() + 1];
        strcpy(miterCNF, fileName.c_str());
        solverResult result = SAT_solver(miterCNF);
        satisfiable = result.satisfiable;
        if(satisfiable){
            for(int i = 0 ; i < result.inputSize ; i++){
                satisfiedInput.push_back((result.input[i] > 0 ? 1 : 0) ^ inv[i]);
            }
            free(result.input);
        }
        return satisfiable;
    }
}

bool CNF::isDC(const string &name) {
#ifdef DBG
    bool reIsDC = (DC.find(name) != DC.end());
    if(reIsDC != (varMap.find(name) == varMap.end())){
        cout << "[CNF] Error: DC determine error." << endl;
        exit(1);
    }
    return reIsDC;
#elif
    return varMap.find(name) == varMap.end();
#endif
}
