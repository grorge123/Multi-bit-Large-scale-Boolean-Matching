//
// Created by grorge on 5/19/23.
//

#include "parser.h"


#include <fstream>

using namespace std;

inputStructure parseInput(string inputPath) {
    inputStructure result;
    ifstream inputFile;
    inputFile.open(inputPath);
    inputFile >> result.cir1AIGPath;
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
    inputFile >> result.cir2AIGPath;
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
    //TODO translate verilog to AIG

    return result;
}
