//
// Created by grorge on 5/19/23.
//
#include <iostream>
#include "utility.h"
#include "parser.h"


#include <fstream>
using namespace std;

extern Abc_Frame_t * pAbc;

InputStructure parseInput(string inputPath) {
    InputStructure result;
    ifstream inputFile;
    inputFile.open(inputPath);
    string cir1VerilogPath, cir2VerilogPath;
    inputFile >> cir1VerilogPath;
    int busNum = 0;
    inputFile >> busNum;
    for(int i = 0 ; i < busNum ; i++){
        int busSz = 0;
        inputFile >> busSz;
        vector<string> busVector;
        for(int q = 0 ; q < busSz ; q++){
            string busName;
            inputFile >> busName;
            busVector.push_back(busName);
        }
        result.cir1Bus.push_back(busVector);
    }
    inputFile >> cir2VerilogPath;
    inputFile >> busNum;
    for(int i = 0 ; i < busNum ; i++){
        int busSz = 0;
        inputFile >> busSz;
        vector<string> busVector;
        for(int q = 0 ; q < busSz ; q++){
            string busName;
            inputFile >> busName;
            busVector.push_back(busName);
        }
        result.cir2Bus.push_back(busVector);
    }
    result.cir1AIGPath = cir1VerilogPath + ".aig";
    result.cir2AIGPath = cir2VerilogPath + ".aig";
    string resultPath = "stdoutOutput.txt";
    cout.flush();
    FILE *saveStdout = stdout;
    stdout = fopen(resultPath.c_str(), "w");
    if (stdout != NULL) {
        string cmd1 = produceABCCommand(cir1VerilogPath, result.cir1AIGPath);
        string cmd2 = produceABCCommand(cir2VerilogPath, result.cir2AIGPath);
        if (Cmd_CommandExecute(pAbc, cmd1.c_str())){
            cout << "[parser] ERROR:Cannot execute command \"" << cmd1 << "\".\n";
            exit(1);
        }
        if (Cmd_CommandExecute(pAbc, cmd2.c_str())){
            cout << "[parser] ERROR:Cannot execute command \"" << cmd2 << "\".\n";
            exit(1);
        }
        fflush(stdout);
        fclose(stdout);
        stdout = saveStdout;
    } else {
        cout << "[parser] ERROR:Can't write file:" << resultPath << endl;
        exit(1);
    }
    return result;
}

string produceABCCommand(string inputPath, string outputPath) {
    //TODO test which command can reduce most network space
    return "read " + inputPath +"; strash; write_aiger -s " + outputPath;
}

void parseOutput(string outputPath, OutputStructure result) {
    ofstream outputFile;
    outputFile.open(outputPath);
    for(auto group : result.inputGroups){
        outputFile << "INGROUP\n";
        outputFile << "1 " << (group.inv ? "-" : "+") << " " << group.cir1 << "\n";
        for(unsigned int i = 0 ; i < group.cir2.size() ; i++){
            outputFile << "2 " << (group.invVector[i] ? "-" : "+") << " " << group.cir2[i] << "\n";
        }
        outputFile << "END\n";
    }
    for(auto group : result.outputGroups){
        outputFile << "OUTGROUP\n";
        outputFile << "1 " << (group.inv ? "-" : "+") << " " << group.cir1 << "\n";
        for(unsigned int i = 0 ; i < group.cir2.size() ; i++){
            outputFile << "2 " << (group.invVector[i] ? "-" : "+") << " " << group.cir2[i] << "\n";
        }
        outputFile << "END\n";
    }
    if(result.one.size() != 0 || result.zero.size() != 0){
        outputFile << "CONSTGROUP\n";
        for(auto group: result.one){
            outputFile << "- " << group << "\n";
        }
        for(auto group: result.zero){
            outputFile << "+ " << group << "\n";
        }
        outputFile << "END\n";
    }
    outputFile.close();
}
