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
    ifstream ifs;
    ifs.open(fileName + ".cnf");
    string input;
    while (ifs >> input){
        if(input[0] != 'c')break;
        stringstream ss;
        ss << input;
        ss >> input;
        int aigIdx, cnfIdx;
        ss >> aigIdx >> input >> cnfIdx;
        if(aig.isInput(aigIdx)){
            // TODO only add input to map table
            varMap.insert(pair<string, int> (aig.fromOrderToName(aigIdx), cnfIdx));
        }
    }
    ifs.close();
    readFromFile(fileName + ".cnf");
}


void CNF::readFromFile(string inputPath) {
    ifstream ifs;
    ifs.open(inputPath);
    string input;
    while (ifs >> input){
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

string CNF::getRow() {
    string raw;
    raw += "p cnf " + to_string(maxIdx) + " " + to_string(clauses.size()) + '\n';
    for(auto clause : clauses){
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

bool CNF::isSatisfied() {
    if(checkSatisfied){
        return satisfied;
    }else{
        string fileName = "CNFSatisfied.cnf";
        string content = getRow();
        ofstream ofs;
        ofs.open(fileName + ".aig");
        ofs << content;
        ofs.close();
        char * miterCNF = new char[fileName.size() + 1];
        strcpy(miterCNF, fileName.c_str());
        solverResult result = SAT_solver(miterCNF);
        satisfied = result.satisfiable;
        for(int i = 0 ; i < result.inputSize ; i++){
            satisfiedInput.push_back(result.input[i]);
        }
        return satisfied;
    }
}
