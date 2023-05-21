//
// Created by grorge on 5/19/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARSER_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARSER_H

#include <string>
#include <vector>
using namespace  std;
struct inputStructure{
    string cir1AIGPath;
    string cir2AIGPath;
    vector<vector<string> > cir1Bus;
    vector<vector<string> > cir2Bus;
};
struct group{
    string cir1;
    vector<string> cir2;
};
struct outputStructure{
    vector<group> inputGroups;
    vector<group> outputGroups;
    vector<string> one, zero;
};
string produceABCCommand(string inputPath, string outputPath);
inputStructure parseInput(string inputPath);
void parseOutput(string outputPath, outputStructure result);
#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_PARSER_H
