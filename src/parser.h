//
// Created by grorge on 5/19/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARSER_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARSER_H

#include <string>
#include <vector>
using namespace  std;
struct InputStructure{
    string cir1AIGPath;
    string cir2AIGPath;
    vector<vector<string> > cir1Bus;
    vector<vector<string> > cir2Bus;
};
struct Group{
    string cir1;
    bool inv = false;
    vector<string> cir2;
    vector<bool> invVector;
};
struct OutputStructure{
    vector<Group> inputGroups;
    vector<Group> outputGroups;
    vector<string> one, zero;
};
string produceABCCommand(string inputPath, string outputPath);
InputStructure parseInput(const string& inputPath);
void parseOutput(string outputPath, OutputStructure result);
#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARSER_H
