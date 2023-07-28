//
// Created by grorge on 4/14/23.
//

#include "utility.h"
#include <fstream>
#include <chrono>
#include <ctime>

#ifndef pAbcMain
#define pAbcMain
Abc_Frame_t * pAbc;
#endif
long long int allOptimize = 0;
long long int lastOptimize = 0;
map<string, long long int> allTimeMap;
map<string, long long int> lastTimeMap;
ifstream* readFile(string fileName){
    ifstream* file = new ifstream;
    file->open(fileName, std::ios::in);
    if(!file->is_open()){
        cout << "Unable open file:" << fileName << endl;
        exit(1);
    }
    return file;
}

string formatTime(time_t inputTime){

    std::tm tm_struct;
    gmtime_r(&inputTime, &tm_struct);

    ostringstream oss;
    oss << put_time(std::localtime(&inputTime), "20%y-%m-%d_%OH:%OM:%OS");
    string str = oss.str();
    return str;
}

string getNowTime(){
    auto now = chrono::system_clock::now();
    time_t time = chrono::system_clock::to_time_t(now);
    return formatTime(time);
}
long long int nowTime() {
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    return nanos;
}

void startOptimize() {
    lastOptimize = nowTime();
}

void stopOptimize() {
    allOptimize += nowTime() - lastOptimize;
}

void startStatistic(const string& name) {
    lastTimeMap[name] = nowTime();
}

void stopStatistic(const string& name) {
    allTimeMap[name] += nowTime() - lastTimeMap[name];
}

void printStatistic() {
    for(const auto& pair : allTimeMap){
        cout << pair.first << ": " << pair.second / 1000000 << endl;
    }
}
